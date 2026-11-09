// Runs only against unique temporary LOCAL queues created by this test.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mq.h>
#include <objbase.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "mbx.h"

void require(bool value, const char* text) { if (!value) throw std::runtime_error(text); }
void success(MBXLONG result, const char* text) {
    if (result != 0) {
        char description[256]; std::snprintf(description, sizeof(description), "%s: %ld (0x%08lX)", text, result, static_cast<unsigned long>(result));
        throw std::runtime_error(description);
    }
}
struct LocalQueue {
    std::string name;
    std::wstring format;
    LocalQueue() {
        GUID guid; require(SUCCEEDED(CoCreateGuid(&guid)), "Create queue GUID");
        name = "mbx_test_";
        const BYTE* guidBytes = reinterpret_cast<const BYTE*>(&guid);
        for (size_t i = 0; i < sizeof(guid); ++i) {
            const BYTE byte = guidBytes[i];
            char hex[3]; std::snprintf(hex, sizeof(hex), "%02x", byte); name += hex;
        }
        std::wstring path = L".\\private$\\" + std::wstring(name.begin(), name.end());
        PROPID ids[] = {PROPID_Q_PATHNAME, PROPID_Q_TRANSACTION};
        MQPROPVARIANT values[2] = {}; HRESULT status[2] = {};
        values[0].vt = VT_LPWSTR; values[0].pwszVal = &path[0];
        values[1].vt = VT_UI1; values[1].bVal = MQ_TRANSACTIONAL_NONE;
        MQQUEUEPROPS props = {2, ids, values, status};
        wchar_t result[1024]; DWORD size = 1024;
        require(SUCCEEDED(MQCreateQueue(nullptr, &props, result, &size)), "Create local queue (MSMQ service/permissions required)");
        format = result;
    }
    ~LocalQueue() { if (!format.empty()) MQDeleteQueue(format.c_str()); }
};
struct Session {
    MBXHMBX connection = 0;
    MBXHMB send = 0, read = 0, browse = 0;
    explicit Session(LocalQueue& queue) {
        char host[] = ".", manager[] = "legacy-manager";
        success(MBXCONNX(&connection, manager, host), "Connect");
        try {
            success(MBXOPEN(connection, &send, &queue.name[0], MBXOO_OUTPUT), "Open send");
            success(MBXOPEN(connection, &read, &queue.name[0], MBXOO_INPUT_SHARED), "Open receive");
            success(MBXOPEN(connection, &browse, &queue.name[0], MBXOO_BROWSE), "Open browse");
        } catch (...) { MBXDISC(&connection); throw; }
    }
    ~Session() { if (connection) MBXDISC(&connection); }
};
int main() {
    try {
        static_assert(sizeof(void*) == 8 && sizeof(MBXMD) == 524, "x64 MBX ABI");
        LocalQueue queue; Session s(queue);
        std::vector<char> body(8192);
        for (size_t i = 0; i < body.size(); ++i) body[i] = static_cast<char>(i & 255);
        char output[8192] = {};
        MBXMD send = {}, received = {};
        send.type = MBXMT_REQUEST; send.exp = -1;
        std::strcpy(send.rmb, queue.name.c_str());
        success(MBXPUT(s.connection, s.send, body.data(), static_cast<MBXLONG>(body.size()), &send), "Put binary request");
        require(send.len == 8192, "PUT returns length");
        success(MBXPEEK(s.connection, s.browse, nullptr, nullptr, output, sizeof(output), MBXPMO_BROWSE_FIRST, 1000, &received), "Peek request");
        require(std::memcmp(output, body.data(), body.size()) == 0, "Binary body roundtrip");
        require(received.type == MBXMT_REQUEST && received.exp == -1, "Metadata type/lifetime");
        require(std::strcmp(received.rmb, queue.name.c_str()) == 0, "Reply queue roundtrip");
        require(std::memcmp(received.msgid1, send.msgid1, 128) == 0, "Generated MsgId");
        require(std::memcmp(received.msgid2, send.msgid2, 128) == 0, "Generated CorrelationId");
        success(MBXGET(s.connection, s.read, &send.msgid1, &send.msgid2, output, sizeof(output), 0, &received), "Get after peek");
        require(MBXGET(s.connection, s.read, nullptr, nullptr, output, sizeof(output), 0, &received) == MBXRC_NO_MSG_AVAILABLE, "Empty queue");

        // Full 128-byte filters must distinguish values sharing the native first 20 bytes.
        MBXMD a = {}, b = {}; a.exp = b.exp = -1;
        a.msgid1[127] = 9; // custom logical MsgId cannot be set as native MSMQ MsgId
        a.msgid2[0] = b.msgid2[0] = 7; a.msgid2[127] = 1; b.msgid2[127] = 2;
        char first[] = "first", second[] = "second";
        success(MBXPUT(s.connection, s.send, first, sizeof(first), &a), "Put A");
        success(MBXPUT(s.connection, s.send, second, sizeof(second), &b), "Put B");
        success(MBXPEEK(s.connection, s.browse, nullptr, nullptr, output, sizeof(output), MBXPMO_BROWSE_FIRST, 1000, &received), "Browse first");
        require(std::strcmp(output, first) == 0, "FIRST returns A");
        success(MBXPEEK(s.connection, s.browse, nullptr, nullptr, output, sizeof(output), MBXPMO_BROWSE_NEXT, 1000, &received), "Browse next");
        require(std::strcmp(output, second) == 0, "NEXT returns B");
        success(MBXGET(s.connection, s.read, nullptr, &b.msgid2, output, sizeof(output), 0, &received), "Filtered receive B");
        require(std::strcmp(output, second) == 0, "Match all 128 bytes");
        success(MBXGET(s.connection, s.read, nullptr, nullptr, output, sizeof(output), 0, &received), "Unmatched A remains");
        require(std::strcmp(output, first) == 0, "Filter did not consume A");
        require(std::memcmp(received.msgid1, a.msgid1, 128) == 0, "Custom 128-byte message ID");

        MBXMD small = {}; small.exp = -1;
        success(MBXPUT(s.connection, s.send, second, sizeof(second), &small), "Put truncation payload");
        require(MBXPEEK(s.connection, s.browse, nullptr, nullptr, output, 2, MBXPMO_BROWSE_FIRST, 1000, &received) == MBXRC_TRUNCATED_MSG_ACCEPTED, "Peek truncation");
        require(received.len == sizeof(second) && std::memcmp(output, second, 2) == 0, "Truncation reports full length");
        require(MBXGET(s.connection, s.read, nullptr, nullptr, output, 2, 0, &received) == MBXRC_TRUNCATED_MSG_ACCEPTED, "Receive truncation");
        require(MBXGET(s.connection, s.read, nullptr, nullptr, output, sizeof(output), 0, &received) == MBXRC_NO_MSG_AVAILABLE, "Truncated GET consumes message");
        require(MBXGET(s.connection, s.read, nullptr, nullptr, output, sizeof(output), 50, &received) == MBXRC_NO_MSG_AVAILABLE, "Finite receive timeout");
        require(MBXGET(s.connection, s.send, nullptr, nullptr, output, sizeof(output), 0, &received) < 0, "Reject wrong access mode");
        require(MBXPUT(s.connection, s.send, nullptr, 1, &small) < 0, "Reject invalid body");

        // Interoperability with a native MSMQ message without the MBX Extension.
        QUEUEHANDLE native = nullptr;
        require(SUCCEEDED(MQOpenQueue(queue.format.c_str(), MQ_SEND_ACCESS, MQ_DENY_NONE, &native)), "Open native sender");
        PROPID nativeId = PROPID_M_BODY;
        MQPROPVARIANT nativeValue = {}; HRESULT nativeStatus = S_OK;
        nativeValue.vt = VT_VECTOR | VT_UI1;
        nativeValue.caub = {sizeof(first), reinterpret_cast<BYTE*>(first)};
        MQMSGPROPS nativeProps = {1, &nativeId, &nativeValue, &nativeStatus};
        const HRESULT nativeSend = MQSendMessage(native, &nativeProps, MQ_NO_TRANSACTION);
        MQCloseQueue(native);
        require(SUCCEEDED(nativeSend) && SUCCEEDED(nativeStatus), "Native send");
        success(MBXGET(s.connection, s.read, nullptr, nullptr, output, sizeof(output), 1000, &received), "Read native MSMQ message");
        require(received.len == sizeof(first) && std::memcmp(output, first, sizeof(first)) == 0, "Native body roundtrip");

        MBXHMB stale = s.read;
        success(MBXCLOSE(s.connection, &s.read), "Close queue");
        require(s.read == 0, "Close clears handle");
        require(MBXGET(s.connection, stale, nullptr, nullptr, output, sizeof(output), 0, &received) < 0, "Reject stale handle");
        success(MBXDISC(&s.connection), "Disconnect");
        require(s.connection == 0, "Disconnect clears handle");
        std::cout << "PASS: MSMQ binary roundtrip, IDs, reply metadata, browse, selection, truncation, timeout, handle validation\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n'; return 1;
    }
}
