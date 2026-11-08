#include "HsmsSock.h"
#include "SecsParser.h"
#include "hsmsmsg.h"
#include "mbxmq.h"
#include "export.h"
#include "dynamic_library.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <stdexcept>
namespace {
uint32_t be32(const unsigned char* p) { return (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|p[3]; }
struct Module {
    void* handle;
    explicit Module(const char* name) : handle(dlopen(name, RTLD_NOW)) { if (!handle) throw std::runtime_error(std::string("Cannot load transaction module: ")+name); }
    ~Module() { if (handle) dlclose(handle); }
};
std::mutex log_mutex;
void log(const std::string& s) { std::lock_guard<std::mutex> lock(log_mutex); std::ofstream f("hsms.log", std::ios::app); f << s << '\n'; }
}
void HsmsSock::OnConnect() {
    char config[1024] = {};
    bool enable_mq = GetConfig(const_cast<char*>("RUNTIME"), const_cast<char*>("ENABLE_MQ"), config) == 0 && std::string(config) == "Y";
    std::atomic<bool> stop{false}, mq_failed{false};
    std::thread watcher;
    // Resolve dependencies before starting threads so partial initialization is safe.
    if (enable_mq) {
        watcher = std::thread([&] {
            try {
                char module[1024] = {}, manager[1024] = {}, connection[1024] = {}, queue[1024] = {};
                if (GetConfig(const_cast<char*>("RUNTIME"), const_cast<char*>("TRANSACTION_DLL"), module) ||
                    GetConfig(const_cast<char*>("MQHOST"), const_cast<char*>("CONF_MQ_MGR"), manager) ||
                    GetConfig(const_cast<char*>("MQHOST"), const_cast<char*>("CONNECTION"), connection) ||
                    GetConfig(const_cast<char*>("MQHOST"), const_cast<char*>("RECEIVE_QUEUE"), queue)) throw std::runtime_error("Missing MQ configuration");
                Module plugin(module);
                using Callback = int (*)(CMQMsg*);
                auto callback = reinterpret_cast<Callback>(dlsym(plugin.handle, "tx_S01F01_QSTD"));
                if (!callback) throw std::runtime_error("Missing tx_S01F01_QSTD export");
                MBXMQ mq;
                if (mq.Conn(manager, connection) != 0) throw std::runtime_error("MQ connection failed (check x64 mbx.dll)");
                while (!stop.load()) {
                    // Legacy MBXGET has no output length. Restrict this path to NUL-terminated text.
                    std::vector<char> data(MQ_MSG_BUFF_LEN + 1, 0);
                    MBXMSGID correlation = {};
                    long rc = mq.Recv(queue, nullptr, &correlation, data.data(), MQ_MSG_BUFF_LEN, 1000);
                    if (rc != 0) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); continue; }
                    CMQMsg msg;
                    msg.m_msglen = static_cast<int>(std::strlen(data.data()));
                    msg.m_msgbuf = new char[static_cast<size_t>(msg.m_msglen)+1]{};
                    std::memcpy(msg.m_msgbuf, data.data(), msg.m_msglen);
                    if (callback(&msg) != 0) log("Transaction callback returned an error");
                }
            } catch (const std::exception& e) { mq_failed = true; log(e.what()); std::cerr << e.what() << '\n'; }
        });
    }
    try {
        for (;;) {
            unsigned char header[HSMS_HEADER_SIZE] = {};
            std::cin.read(reinterpret_cast<char*>(header), sizeof(header));
            if (std::cin.gcount() == 0 && std::cin.eof()) break;
            if (std::cin.gcount() != sizeof(header)) throw std::runtime_error("Truncated HSMS header");
            uint32_t size = be32(header);
            if (size < HSMS_HDR_LEN || size > 16u*1024u*1024u) throw std::runtime_error("Invalid HSMS message length");
            if (header[8] != 0) throw std::runtime_error("Unsupported HSMS PType");
            std::vector<unsigned char> frame(size + 4);
            std::memcpy(frame.data(), header, sizeof(header));
            size_t body = size - HSMS_HDR_LEN;
            if (body) {
                std::cin.read(reinterpret_cast<char*>(frame.data()+HSMS_HEADER_SIZE), body);
                if (static_cast<size_t>(std::cin.gcount()) != body) throw std::runtime_error("Truncated HSMS body");
            }
            if (header[9]) {
                if (size != 10) throw std::runtime_error("Control frame must have no body");
                if (header[9] == 9) break; // Separate.req; header index 9 is not itself an SType value.
                if (header[9] == 1 || header[9] == 5) {
                    ++header[9]; header[6] = header[7] = 0;
                    std::cout.write(reinterpret_cast<char*>(header), sizeof(header)); std::cout.flush();
                    if (!std::cout) throw std::runtime_error("HSMS output failed");
                } else log("Control frame received, SType="+std::to_string(header[9]));
            } else {
                SecsParser parser(reinterpret_cast<char*>(frame.data()), static_cast<int>(frame.size()));
                log(parser.GetXML());
                // Original SECS business callback was commented out; do not invent a transaction response.
            }
        }
    } catch (...) { stop = true; if (watcher.joinable()) watcher.join(); throw; }
    stop = true;
    if (watcher.joinable()) watcher.join();
    if (mq_failed) throw std::runtime_error("MQ worker failed; see hsms.log");
}
