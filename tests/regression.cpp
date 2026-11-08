#include "SecsParser.h"
#include "TCSQueue.h"
#include "memlist.h"
#include "smldoc.h"
#include "xmldoc.h"
#include "mbxmq.h"
#include "export.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <thread>
#include <atomic>
#include <sstream>
#define CHECK(x) do { if (!(x)) throw std::runtime_error("Failed: " #x); } while (0)
static std::vector<char> frame(std::initializer_list<unsigned char> body) {
    std::vector<char> v(14, 0); unsigned n = 10 + static_cast<unsigned>(body.size());
    for (int i = 0; i < 4; ++i) v[3-i] = static_cast<char>(n >> (i*8));
    v[6] = static_cast<char>(0xff); v[7] = 1;
    v[10] = static_cast<char>(0xfe); v[11] = 0x12; v[12] = 0x34; v[13] = 0x56;
    for (unsigned char c : body) v.push_back(static_cast<char>(c)); return v;
}
static std::string parse(std::vector<char> v) { return SecsParser(v.data(), static_cast<int>(v.size())).GetXML(); }
static void rejects(std::vector<char> v) { bool caught = false; try { parse(v); } catch (const std::runtime_error&) { caught = true; } CHECK(caught); }
class CaptureOutput {
    std::streambuf* previous_;
    std::ios::iostate state_;
public:
    std::ostringstream bytes;
    CaptureOutput() : previous_(std::cout.rdbuf()), state_(std::cout.rdstate()) { std::cout.rdbuf(bytes.rdbuf()); std::cout.clear(); }
    ~CaptureOutput() { std::cout.rdbuf(previous_); std::cout.clear(state_); }
};
static void test_transmission() {
    CaptureOutput capture;
    CHSMSMsg message, request;
    std::strcpy(message.m_sfname, "S127F255");
    CHECK(SendRecvHSMS(&message, nullptr) == 0);
    const std::string sent = capture.bytes.str();
    CHECK(sent.size() == 14 && static_cast<unsigned char>(sent[6]) == 255 && static_cast<unsigned char>(sent[7]) == 255);
    CHECK(static_cast<unsigned char>(sent[3]) == 10);
    capture.bytes.str("");
    std::memset(message.m_sfname, '9', sizeof(message.m_sfname));
    CHECK(SendRecvHSMS(&message, nullptr) == -1);
    std::strcpy(message.m_sfname, "S999999999999999999999999F1");
    CHECK(SendRecvHSMS(&message, nullptr) == -1);
    std::strcpy(message.m_sfname, "S1F1trailing");
    CHECK(SendRecvHSMS(&message, nullptr) == -1);
    CHECK(capture.bytes.str().empty());
    request.m_hsmshdr[7] = 255;
    CHECK(ReplyHSMS(&request, &message) == -1);
    request.m_hsmshdr[6] = 0xff; request.m_hsmshdr[7] = 1;
    CHECK(ReplyHSMS(&request, &message) == 0);
    CHECK(static_cast<unsigned char>(capture.bytes.str()[6]) == 127);
    CHECK(static_cast<unsigned char>(capture.bytes.str()[7]) == 2);
    std::cout.setstate(std::ios::badbit);
    std::strcpy(message.m_sfname, "S1F1");
    CHECK(SendRecvHSMS(&message, nullptr) == -1);
    CHECK(ReplyHSMS(&request, &message) == -1);
}
int main() {
 try {
    test_transmission();
    CHECK(sizeof(MBXLONG) == 4);
    CHECK(parse(frame({})).find("S127F1") != std::string::npos);
    CHECK(parse(frame({})).find("4262605910") != std::string::npos);
    CHECK(parse(frame({0x41,3,'A','&','B'})).find("A&amp;B") != std::string::npos);
    CHECK(parse(frame({0x01,0})).find("</LV_1_IT_1>") != std::string::npos);
    CHECK(parse(frame({0x01,2,0x41,0,0xa5,1,255})).find("255 ") != std::string::npos);
    CHECK(parse(frame({0x72,0,4,0xff,0xff,0xff,0xff})).find("-1 ") != std::string::npos);
    CHECK(parse(frame({0x91,4,0x3f,0x80,0,0})).find("value=\"1 ") != std::string::npos);
    CHECK(parse(frame({0x25,1,1})).find("value=\"T ") != std::string::npos);
    rejects({}); rejects(frame({0x40,0})); rejects(frame({0xff,0}));
    rejects(frame({0x41,5,'X'})); rejects(frame({0x01,2,0x41,0}));
    rejects(frame({0x71,3,1,2,3})); rejects(frame({0x41,0,0x41,0}));
    auto mismatch = frame({}); mismatch[3] = 11; rejects(mismatch);
    // Historical overflow: ASCII greater than the old 1024-byte temporary buffer.
    auto large = frame({}); large.insert(large.end(), {0x42,8,0}); large.insert(large.end(), 2048, 'X');
    unsigned n = static_cast<unsigned>(large.size()-4); for (int i=0;i<4;++i) large[3-i] = static_cast<char>(n>>(i*8));
    CHECK(parse(large).size() > 2048);
    CMemList list; int data = 123; void* member = nullptr;
    void* h = list.Add(&data, sizeof(data), &member); CHECK(h && member && list.Count()==1);
    CHECK(*static_cast<int*>(list.Member(h)) == 123); CHECK(list.Delete(h)==0); CHECK(list.Count()==0); list.Clear(); list.Clear();
    CXMLDoc null_doc; CXMLDoc named_doc(const_cast<char*>("TEST"));
    CSMLDoc sml(const_cast<char*>("S01F01")); CHSMSMsg msg(0);
    CHECK(sml.SetString(const_cast<char*>("ABC"), const_cast<char*>("NAME")) == 0);
    CHECK(sml.Build(&msg) == 0); CHECK(msg.m_bodylen == 5); CHECK(msg.m_bodybuf[0] == 0x41 && msg.m_bodybuf[1] == 3);
    CHECK(sml.Build(&msg) == 0); // repeated build releases the old buffer
    msg.SetWaitBit(); msg.SetWaitBit(); CHECK(msg.m_hsmshdr[HSMS_HDR_BYTE_2] == 0x80);
    TCSQueue a(SECS_QUEUE), b(TRX_QUEUE); char x[]="ABC", out[16]{};
    CHECK(a.Put(3,x)==0); CHECK(b.Count()==0); CHECK(a.Get(2,out)==-1); CHECK(a.Count()==1);
    CHECK(a.Get(16,out)==0 && std::string(out)=="ABC"); CHECK(a.Count()==0);
    CHECK(a.Put(a.GetMaxUnitSize()+1,x)==-1); CHECK(a.Put(-1,x)==-1);
    for(long i=0;i<a.GetMaxDeepth();++i) CHECK(a.Put(3,x)==0);
    CHECK(a.Put(3,x)==-1); a.Clear();
    // FIFO and producer/consumer contention including wraparound equivalents.
    std::atomic<bool> failed{false};
    std::thread producer([&] { for(int i=0;i<10000;++i) while(a.Put(sizeof(i),reinterpret_cast<char*>(&i))) std::this_thread::yield(); });
    for(int i=0;i<10000;++i) { int got = -1; while(a.Get(sizeof(got),reinterpret_cast<char*>(&got))) std::this_thread::yield(); if(got!=i) failed=true; }
    producer.join(); CHECK(!failed);
    MBXMQ missing(const_cast<char*>("./definitely-missing-library"));
    CHECK(missing.Conn(const_cast<char*>("QM"),const_cast<char*>("HOST"))!=0);
    CHECK(missing.Open(const_cast<char*>("Q"),0)!=0);
    char config[1024]{};
    CHECK(GetConfig(const_cast<char*>("SECS_QUEUE"),const_cast<char*>("MAX_DEEPTH"),config)==0);
    CHECK(std::string(config)=="5");
    char small[1] = {'X'};
    CHECK(GetConfigSized("SECS_QUEUE", "MAX_DEEPTH", small, sizeof(small)) == -1);
    CHECK(small[0] == 0);
    CHECK(GetConfig(const_cast<char*>("SECS_QUEUE"),const_cast<char*>("NOT_A_KEY"),config)!=0);
    CHECK(config[0]==0);
    std::cout << "All HSMS regression groups passed\n"; return 0;
 } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
