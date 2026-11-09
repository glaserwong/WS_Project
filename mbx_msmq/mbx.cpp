// MSMQ implementation of the original MBX C ABI. Windows 11 x64 / MSVC.
// Queue contents are binary; MBX metadata is carried in MSMQ Extension.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#include <mq.h>
#include <objbase.h>
#include <cwchar>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "mbx.h"

static_assert(sizeof(void*) == 8, "Build the x64 configuration");
static_assert(sizeof(MBXLONG) == 4 && sizeof(MBXMD) == 524, "MBX ABI mismatch");

namespace {
constexpr DWORD NativeIdSize = 20;
constexpr DWORD MaxMessage = 4 * 1024 * 1024;
constexpr MBXLONG InvalidHandle = static_cast<MBXLONG>(MQ_ERROR_INVALID_HANDLE);
constexpr MBXLONG InvalidArgument = static_cast<MBXLONG>(E_INVALIDARG);
constexpr MBXLONG MetadataTooLong = -10001;
constexpr MBXLONG InvalidBrowseState = -10002;
constexpr BYTE WireMagic[8] = {'M','B','X','M','S','M','Q',1};
// Explicit byte offsets avoid compiler packing and pointer-width dependencies.
constexpr DWORD WireSize = 8 + 128 * 4 + 4 * 2;

struct Failure { HRESULT code; };
void check(HRESULT hr) { if (FAILED(hr)) throw Failure{hr}; }
MBXLONG error() noexcept {
    try { throw; }
    catch (const Failure& f) { return static_cast<MBXLONG>(f.code); }
    catch (const std::bad_alloc&) { return static_cast<MBXLONG>(E_OUTOFMEMORY); }
    catch (...) { return static_cast<MBXLONG>(E_INVALIDARG); }
}
struct Lock {
    SRWLOCK* native;
    explicit Lock(SRWLOCK& lock) : native(&lock) { AcquireSRWLockExclusive(native); }
    ~Lock() { ReleaseSRWLockExclusive(native); }
    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;
};
struct Cursor {
    HANDLE value = nullptr;
    explicit Cursor(QUEUEHANDLE queue) { check(MQCreateCursor(queue, &value)); }
    ~Cursor() { if (value) MQCloseCursor(value); }
};
struct Connection { std::wstring host; };
struct Queue {
    QUEUEHANDLE native = nullptr;
    MBXHMBX owner = 0;
    MBXLONG mode = 0;
    std::atomic<bool> closed{false};
    SRWLOCK browseLock = SRWLOCK_INIT;
    std::unique_ptr<Cursor> browse;
    ~Queue() { browse.reset(); if (native) MQCloseQueue(native); }
};
struct Registry {
    SRWLOCK lock = SRWLOCK_INIT;
    std::uint64_t next = 1;
    std::map<MBXHMBX, std::shared_ptr<Connection>> connections;
    std::map<MBXHMB, std::shared_ptr<Queue>> queues;
    MBXLONG id() {
        if (next > LONG_MAX) throw Failure{E_OUTOFMEMORY};
        return static_cast<MBXLONG>(next++);
    }
};
Registry& registry() { static Registry value; return value; }
std::shared_ptr<Queue> findQueue(MBXHMBX owner, MBXHMB id) {
    auto& r = registry(); Lock guard(r.lock);
    auto it = r.queues.find(id);
    if (!r.connections.count(owner) || it == r.queues.end() ||
        it->second->owner != owner || it->second->closed.load())
        throw Failure{MQ_ERROR_INVALID_HANDLE};
    return it->second;
}
std::wstring wide(const std::string& value) {
    if (value.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                static_cast<int>(value.size()), nullptr, 0);
    if (!n) throw Failure{E_INVALIDARG};
    std::wstring result(n, L'\0');
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                             static_cast<int>(value.size()), &result[0], n))
        throw Failure{E_INVALIDARG};
    return result;
}
std::string utf8(const std::wstring& value) {
    if (value.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                                static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (!n) throw Failure{E_INVALIDARG};
    std::string result(n, '\0');
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                             static_cast<int>(value.size()), &result[0], n, nullptr, nullptr))
        throw Failure{E_INVALIDARG};
    return result;
}
std::string field(const char* value, size_t size) {
    const char* end = static_cast<const char*>(std::memchr(value, 0, size));
    if (!end) throw Failure{E_INVALIDARG};
    return std::string(value, end);
}
std::wstring hostName(const char* value) {
    auto host = wide(value && *value ? value : ".");
    auto port = host.find(L'('); // Accept old IBM-style host(port); MSMQ has no such port.
    if (port != std::wstring::npos) {
        if (host.back() != L')' || port == 0) throw Failure{E_INVALIDARG};
        host.resize(port);
    }
    if (host.empty() || host.find_first_of(L"\\/=:;") != std::wstring::npos)
        throw Failure{E_INVALIDARG};
    return host;
}
bool starts(const std::wstring& value, const wchar_t* prefix) {
    return value.compare(0, std::wcslen(prefix), prefix) == 0;
}
std::wstring formatName(const std::wstring& host, const std::wstring& name) {
    if (name.empty()) throw Failure{E_INVALIDARG};
    auto format = starts(name, L"FORMATNAME:") ? name.substr(11) : name;
    if (starts(format, L"DIRECT=") || starts(format, L"PUBLIC=") || starts(format, L"PRIVATE="))
        return format;
    if (format.find(L'\\') == std::wstring::npos)
        format = host + L"\\private$\\" + format;
    else if (starts(format, L"private$\\")) format = host + L"\\" + format;
    if (starts(format, L".\\")) {
        wchar_t computer[MAX_COMPUTERNAME_LENGTH + 1]; DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        if (!GetComputerNameW(computer, &size)) throw Failure{HRESULT_FROM_WIN32(GetLastError())};
        format.replace(0, 1, computer, size);
    }
    return L"DIRECT=OS:" + format;
}
bool hasId(PMBXMSGID id) {
    return id && std::any_of(*id, *id + MBX_MSG_ID_LENGTH, [](char b) { return b != 0; });
}
void put32(BYTE* buffer, MBXLONG value) {
    auto n = static_cast<std::uint32_t>(value);
    for (int i = 0; i < 4; ++i) buffer[i] = static_cast<BYTE>(n >> (8 * i));
}
MBXLONG get32(const BYTE* buffer) {
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) value |= std::uint32_t(buffer[i]) << (8 * i);
    return static_cast<MBXLONG>(value);
}
std::vector<BYTE> encode(const MBXMD& md) {
    std::vector<BYTE> result(WireSize);
    std::memcpy(result.data(), WireMagic, 8);
    std::memcpy(result.data() + 8, md.rmbm, 128);
    std::memcpy(result.data() + 136, md.rmb, 128);
    std::memcpy(result.data() + 264, md.msgid1, 128);
    std::memcpy(result.data() + 392, md.msgid2, 128);
    put32(result.data() + 520, md.type);
    put32(result.data() + 524, md.exp);
    return result;
}

// Own every property buffer throughout the synchronous MSMQ call.
struct Message {
    enum { Body, BodyLen, Extension, ExtensionLen, Id, Correlation, Lookup,
           App, Lifetime, Response, ResponseLen, Count };
    PROPID ids[Count] = {PROPID_M_BODY, PROPID_M_BODY_SIZE, PROPID_M_EXTENSION,
        PROPID_M_EXTENSION_LEN, PROPID_M_MSGID, PROPID_M_CORRELATIONID,
        PROPID_M_LOOKUPID, PROPID_M_APPSPECIFIC, PROPID_M_TIME_TO_BE_RECEIVED,
        PROPID_M_RESP_QUEUE, PROPID_M_RESP_QUEUE_LEN};
    MQPROPVARIANT values[Count] = {};
    HRESULT status[Count] = {};
    MQMSGPROPS props = {};
    std::vector<BYTE> body = std::vector<BYTE>(4096);
    std::vector<BYTE> extension = std::vector<BYTE>(WireSize);
    BYTE id[NativeIdSize] = {}, correlation[NativeIdSize] = {};
    std::vector<wchar_t> response = std::vector<wchar_t>(1024);
    void prepare() {
        std::memset(values, 0, sizeof(values)); std::memset(status, 0, sizeof(status));
        values[Body].vt = VT_VECTOR | VT_UI1;
        values[Body].caub = {static_cast<ULONG>(body.size()), body.data()};
        values[BodyLen].vt = VT_UI4;
        values[Extension].vt = VT_VECTOR | VT_UI1;
        values[Extension].caub = {static_cast<ULONG>(extension.size()), extension.data()};
        values[ExtensionLen].vt = VT_UI4;
        values[Id].vt = values[Correlation].vt = VT_VECTOR | VT_UI1;
        values[Id].caub = {NativeIdSize, id};
        values[Correlation].caub = {NativeIdSize, correlation};
        values[Lookup].vt = VT_UI8;
        values[App].vt = values[Lifetime].vt = VT_UI4;
        values[Response].vt = VT_LPWSTR; values[Response].pwszVal = response.data();
        values[ResponseLen].vt = VT_UI4;
        values[ResponseLen].ulVal = static_cast<ULONG>(response.size());
        props = {Count, ids, values, status};
    }
    bool grow() {
        const ULONG b = values[BodyLen].ulVal, e = values[ExtensionLen].ulVal;
        const ULONG r = values[ResponseLen].ulVal;
        if (b > MaxMessage || e > MaxMessage || r > 32768) throw Failure{MQ_ERROR_BUFFER_OVERFLOW};
        bool changed = false;
        if (b > body.size()) { body.resize(b); changed = true; }
        if (e > extension.size()) { extension.resize(e); changed = true; }
        if (r > response.size()) { response.resize(r); changed = true; }
        return changed;
    }
    MBXMD metadata() const {
        MBXMD md = {};
        std::memcpy(md.msgid1, id, NativeIdSize);
        std::memcpy(md.msgid2, correlation, NativeIdSize);
        const auto life = values[Lifetime].ulVal;
        md.exp = (life == LONG_LIVED || life == MQ_INFINITE) ? -1 : static_cast<MBXLONG>(
            std::min<std::uint64_t>(std::uint64_t(life) * 1000, LONG_MAX));
        md.type = values[App].ulVal <= MBXMT_REPLY ? values[App].ulVal : MBXMT_NONE;
        if (values[ExtensionLen].ulVal == WireSize &&
            std::memcmp(extension.data(), WireMagic, 8) == 0) {
            std::memcpy(md.rmbm, extension.data() + 8, 128);
            std::memcpy(md.rmb, extension.data() + 136, 128);
            MBXMSGID supplied = {};
            std::memcpy(supplied, extension.data() + 264, 128);
            if (hasId(&supplied)) std::memcpy(md.msgid1, supplied, 128);
            std::memcpy(md.msgid2, extension.data() + 392, 128);
            md.type = get32(extension.data() + 520);
            md.exp = get32(extension.data() + 524); // requested MBX lifetime, not remaining time
        } else if (values[ResponseLen].ulVal > 1) {
            auto reply = utf8(response.data());
            if (reply.size() >= sizeof(md.rmb)) throw MetadataTooLong;
            std::memcpy(md.rmb, reply.data(), reply.size());
        }
        md.len = static_cast<MBXLONG>(values[BodyLen].ulVal);
        return md;
    }
};
struct Deadline {
    bool infinite;
    std::chrono::steady_clock::time_point until;
    explicit Deadline(MBXLONG ms) : infinite(ms < 0),
        until(std::chrono::steady_clock::now() + std::chrono::milliseconds(ms < 0 ? 0 : ms)) {}
    bool expired() const { return !infinite && std::chrono::steady_clock::now() >= until; }
    DWORD slice() const {
        if (infinite) return 100;
        auto left = std::chrono::duration_cast<std::chrono::milliseconds>(until - std::chrono::steady_clock::now()).count();
        return static_cast<DWORD>(std::max<long long>(0, std::min<long long>(100, left)));
    }
};

MBXLONG receive(MBXHMBX owner, MBXHMB handle, PMBXMSGID matchId, PMBXMSGID matchCorrelation,
                char* buffer, MBXLONG length, MBXLONG timer, MBXMD* output, bool peek, MBXLONG mode) {
    if (!output || length < 0 || (length && !buffer)) return InvalidArgument;
    // Filters can alias output fields; capture them before clearing output.
    MBXMSGID id = {}, correlation = {};
    const bool useId = hasId(matchId), useCorrelation = hasId(matchCorrelation);
    if (useId) std::memcpy(id, *matchId, sizeof(id));
    if (useCorrelation) std::memcpy(correlation, *matchCorrelation, sizeof(correlation));
    std::memset(output, 0, sizeof(*output));
    try {
        auto queue = findQueue(owner, handle);
        if ((peek && queue->mode != MBXOO_BROWSE) || (!peek && queue->mode != MBXOO_INPUT_SHARED))
            return InvalidArgument;
        if (peek && mode != MBXPMO_BROWSE_FIRST && mode != MBXPMO_BROWSE_NEXT) return InvalidArgument;
        std::unique_ptr<Lock> browseGuard;
        std::unique_ptr<Cursor> local;
        HANDLE cursor;
        DWORD action = MQ_ACTION_PEEK_CURRENT;
        if (peek) {
            browseGuard = std::make_unique<Lock>(queue->browseLock);
            if (mode == MBXPMO_BROWSE_FIRST) queue->browse = std::make_unique<Cursor>(queue->native);
            else if (!queue->browse) return InvalidBrowseState;
            else action = MQ_ACTION_PEEK_NEXT;
            cursor = queue->browse->value;
        } else {
            local = std::make_unique<Cursor>(queue->native); cursor = local->value;
        }
        Deadline deadline(timer);
        Message message;
        bool firstAttempt = true;
        for (;;) {
            if (queue->closed.load()) return InvalidHandle;
            // timer=0 still scans all existing candidates without waiting.
            if (timer != 0 && !firstAttempt && deadline.expired()) return MBXRC_NO_MSG_AVAILABLE;
            firstAttempt = false;
            message.prepare();
            HRESULT hr = MQReceiveMessage(queue->native, deadline.slice(), action,
                &message.props, nullptr, nullptr, cursor, MQ_NO_TRANSACTION);
            if (hr == MQ_ERROR_ILLEGAL_CURSOR_ACTION && !peek) {
                local = std::make_unique<Cursor>(queue->native);
                cursor = local->value; action = MQ_ACTION_PEEK_CURRENT;
                continue;
            }
            if (hr == MQ_ERROR_IO_TIMEOUT) {
                if (timer == 0 || deadline.expired()) return MBXRC_NO_MSG_AVAILABLE;
                continue;
            }
            if (hr == MQ_ERROR_BUFFER_OVERFLOW) {
                if (!message.grow()) return static_cast<MBXLONG>(hr);
                action = MQ_ACTION_PEEK_CURRENT; // NEXT already positioned at this message
                continue;
            }
            check(hr);
            for (HRESULT property : message.status) check(property);
            MBXMD metadata = message.metadata();
            action = MQ_ACTION_PEEK_NEXT;
            if ((useId && std::memcmp(id, metadata.msgid1, 128) != 0) ||
                (useCorrelation && std::memcmp(correlation, metadata.msgid2, 128) != 0)) continue;
            if (!peek) {
                PROPID propId = PROPID_M_LOOKUPID;
                MQPROPVARIANT propValue = {}; propValue.vt = VT_UI8;
                HRESULT status = S_OK;
                MQMSGPROPS removal = {1, &propId, &propValue, &status};
                hr = MQReceiveMessageByLookupId(queue->native, message.values[Message::Lookup].uhVal.QuadPart,
                    MQ_LOOKUP_RECEIVE_CURRENT, &removal, nullptr, nullptr, MQ_NO_TRANSACTION);
                if (hr == MQ_ERROR_MESSAGE_NOT_FOUND) continue; // competing consumer won
                check(hr); check(status);
            }
            *output = metadata;
            const auto copied = std::min<MBXLONG>(length, metadata.len);
            if (copied) std::memcpy(buffer, message.body.data(), static_cast<size_t>(copied));
            return metadata.len > length ? MBXRC_TRUNCATED_MSG_ACCEPTED : 0;
        }
    } catch (MBXLONG code) { return code; }
    catch (...) { return error(); }
}
} // namespace

extern "C" MBXLONG MBXCONNX(PMBXHMBX hmbm, PMBXCHAR mbm, PMBXCHAR cnn) {
    if (!hmbm) return InvalidArgument;
    *hmbm = 0;
    try {
        (void)mbm; // MSMQ has no IBM Queue Manager/channel. Kept for source/ABI compatibility.
        auto connection = std::make_shared<Connection>(); connection->host = hostName(cnn);
        auto& r = registry(); Lock guard(r.lock);
        const auto id = r.id(); r.connections.emplace(id, connection); *hmbm = id; return 0;
    } catch (...) { return error(); }
}
extern "C" MBXLONG MBXDISC(PMBXHMBX hmbm) {
    if (!hmbm) return InvalidArgument;
    try {
        std::vector<std::shared_ptr<Queue>> closing;
        auto& r = registry();
        {
            Lock guard(r.lock);
            if (!r.connections.count(*hmbm)) return InvalidHandle;
            closing.reserve(r.queues.size()); // allocate before changing registry state
            r.connections.erase(*hmbm);
            for (auto it = r.queues.begin(); it != r.queues.end();) {
                if (it->second->owner == *hmbm) {
                    closing.push_back(it->second); it->second->closed.store(true);
                    it = r.queues.erase(it);
                } else ++it;
            }
            *hmbm = 0;
        }
        return 0;
    } catch (...) { return error(); }
}
extern "C" MBXLONG MBXOPEN(MBXHMBX hmbm, PMBXHMB hmb, PMBXCHAR mb, MBXLONG mode) {
    if (!hmb || !mb || (mode != MBXOO_INPUT_SHARED && mode != MBXOO_OUTPUT && mode != MBXOO_BROWSE))
        return InvalidArgument;
    *hmb = 0;
    try {
        auto& r = registry(); std::wstring host;
        { Lock guard(r.lock); auto it = r.connections.find(hmbm);
          if (it == r.connections.end()) return InvalidHandle;
          host = it->second->host; }
        auto format = formatName(host, wide(mb));
        auto queue = std::make_shared<Queue>(); queue->owner = hmbm; queue->mode = mode;
        DWORD access = mode == MBXOO_OUTPUT ? MQ_SEND_ACCESS : mode == MBXOO_BROWSE ? MQ_PEEK_ACCESS : MQ_RECEIVE_ACCESS;
        check(MQOpenQueue(format.c_str(), access, MQ_DENY_NONE, &queue->native));
        Lock guard(r.lock);
        if (!r.connections.count(hmbm)) return InvalidHandle;
        const auto id = r.id(); r.queues.emplace(id, queue); *hmb = id; return 0;
    } catch (...) { return error(); }
}
extern "C" MBXLONG MBXCLOSE(MBXHMBX hmbm, PMBXHMB hmb) {
    if (!hmb) return InvalidArgument;
    try {
        std::shared_ptr<Queue> closing;
        auto& r = registry();
        { Lock guard(r.lock); auto it = r.queues.find(*hmb);
          if (it == r.queues.end() || it->second->owner != hmbm || !r.connections.count(hmbm)) return InvalidHandle;
          closing = it->second; closing->closed.store(true); r.queues.erase(it); *hmb = 0; }
        return 0;
    } catch (...) { return error(); }
}
extern "C" MBXLONG MBXPEEK(MBXHMBX hmbm, MBXHMB hmb, PMBXMSGID msgid1, PMBXMSGID msgid2,
                          PMBXCHAR buf, MBXLONG len, MBXLONG mode, MBXLONG timer, PMBXMD md) {
    return receive(hmbm, hmb, msgid1, msgid2, buf, len, timer, md, true, mode);
}
extern "C" MBXLONG MBXGET(MBXHMBX hmbm, MBXHMB hmb, PMBXMSGID msgid1, PMBXMSGID msgid2,
                         PMBXCHAR buf, MBXLONG len, MBXLONG timer, PMBXMD md) {
    return receive(hmbm, hmb, msgid1, msgid2, buf, len, timer, md, false, 0);
}
extern "C" MBXLONG MBXPUT(MBXHMBX hmbm, MBXHMB hmb, PMBXCHAR buf, MBXLONG len, PMBXMD md) {
    if (!md || len < 0 || static_cast<DWORD>(len) > MaxMessage - WireSize || (len && !buf)) return InvalidArgument;
    if (md->type < MBXMT_NONE || md->type > MBXMT_REPLY) return InvalidArgument;
    try {
        auto queue = findQueue(hmbm, hmb);
        if (queue->mode != MBXOO_OUTPUT) return InvalidArgument;
        auto replyManager = field(md->rmbm, sizeof(md->rmbm));
        auto replyQueue = field(md->rmb, sizeof(md->rmb));
        auto extension = encode(*md);
        std::wstring response;
        if (!replyQueue.empty()) {
            std::wstring host;
            if (replyManager.empty()) {
                auto& r = registry(); Lock guard(r.lock); auto it = r.connections.find(hmbm);
                if (it == r.connections.end()) return InvalidHandle;
                host = it->second->host;
            } else host = hostName(replyManager.c_str());
            response = formatName(host, wide(replyQueue));
        }
        PROPID ids[8] = {PROPID_M_BODY, PROPID_M_BODY_TYPE, PROPID_M_EXTENSION,
            PROPID_M_CORRELATIONID, PROPID_M_APPSPECIFIC, PROPID_M_TIME_TO_BE_RECEIVED,
            PROPID_M_MSGID, PROPID_M_RESP_QUEUE};
        MQPROPVARIANT values[8] = {}; HRESULT status[8] = {};
        BYTE nativeId[NativeIdSize] = {}, correlation[NativeIdSize] = {};
        // IBM MQ NEW_CORREL_ID semantics: every request gets a fresh correlation.
        MBXMSGID generatedCorrelation = {};
        if (md->type == MBXMT_REQUEST) {
            GUID guid; check(CoCreateGuid(&guid));
            std::memcpy(generatedCorrelation, &guid, sizeof(guid));
            std::memcpy(extension.data() + 392, generatedCorrelation, sizeof(generatedCorrelation));
            std::memcpy(correlation, generatedCorrelation, NativeIdSize);
        } else std::memcpy(correlation, md->msgid2, NativeIdSize);
        values[0].vt = VT_VECTOR | VT_UI1;
        values[0].caub = {static_cast<ULONG>(len), reinterpret_cast<BYTE*>(buf)};
        values[1].vt = VT_UI4; values[1].ulVal = VT_VECTOR | VT_UI1;
        values[2].vt = VT_VECTOR | VT_UI1;
        values[2].caub = {static_cast<ULONG>(extension.size()), extension.data()};
        values[3].vt = VT_VECTOR | VT_UI1; values[3].caub = {NativeIdSize, correlation};
        values[4].vt = VT_UI4; values[4].ulVal = md->type;
        values[5].vt = VT_UI4;
        values[5].ulVal = md->exp < 0 ? LONG_LIVED : static_cast<ULONG>((std::uint64_t(md->exp) + 999) / 1000);
        values[6].vt = VT_VECTOR | VT_UI1; values[6].caub = {NativeIdSize, nativeId};
        values[7].vt = VT_LPWSTR; values[7].pwszVal = response.empty() ? nullptr : &response[0];
        MQMSGPROPS props = {response.empty() ? 7UL : 8UL, ids, values, status};
        if (queue->closed.load()) return InvalidHandle;
        check(MQSendMessage(queue->native, &props, MQ_NO_TRANSACTION));
        for (DWORD i = 0; i < props.cProp; ++i) check(status[i]);
        // Return generated identifiers to make reply correlation usable by the caller.
        if (!hasId(&md->msgid1)) { std::memset(md->msgid1, 0, 128); std::memcpy(md->msgid1, nativeId, NativeIdSize); }
        if (hasId(&generatedCorrelation)) std::memcpy(md->msgid2, generatedCorrelation, 128);
        md->len = len;
        return 0;
    } catch (...) { return error(); }
}
