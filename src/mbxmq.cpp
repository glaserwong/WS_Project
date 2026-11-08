#include "mbxmq.h"
#include "dynamic_library.h"
#include <cstring>
#include <memory>
namespace {
template<class F> F symbol(void* h, const char* name) { return h ? reinterpret_cast<F>(dlsym(h, name)) : nullptr; }
using Connect = MBXLONG (*)(PMBXHMBX, PMBXCHAR, PMBXCHAR);
using Disconnect = MBXLONG (*)(PMBXHMBX);
using OpenQueue = MBXLONG (*)(MBXHMBX, PMBXHMB, PMBXCHAR, MBXLONG);
using CloseQueue = MBXLONG (*)(MBXHMBX, PMBXHMB);
using Get = MBXLONG (*)(MBXHMBX, MBXHMB, PMBXMSGID, PMBXMSGID, PMBXCHAR, MBXLONG, MBXLONG, PMBXMD);
using Put = MBXLONG (*)(MBXHMBX, MBXHMB, PMBXCHAR, MBXLONG, PMBXMD);
}
MBXMQ::MBXMQ() : con_handle(nullptr), que_handle(nullptr), so_handle(nullptr), error_msg(nullptr) {
#ifdef _WIN32
    SetSO(const_cast<char*>(".\\mbx.dll"));
#else
    SetSO(const_cast<char*>("./mbx.so"));
#endif
}
MBXMQ::MBXMQ(char* file) : con_handle(nullptr), que_handle(nullptr), so_handle(nullptr), error_msg(nullptr) { SetSO(file); }
MBXMQ::~MBXMQ() { Close(); Disc(); if (so_handle) dlclose(so_handle); }
long MBXMQ::SetSO(char* file) {
    if (!file || con_handle || que_handle) return _ERROR;
    if (so_handle) dlclose(so_handle);
    so_handle = dlopen(file, RTLD_NOW); return so_handle ? _NORMAL : _ERROR;
}
long MBXMQ::Conn(PMBXCHAR manager, PMBXCHAR connection) {
    Close(); Disc();
    std::unique_ptr<MBXHMBX> h(new MBXHMBX{});
    long rc = Conn(h.get(), manager, connection);
    if (!rc) con_handle = h.release(); return rc;
}
long MBXMQ::Conn(MBXHMBX* h, PMBXCHAR manager, PMBXCHAR connection) {
    auto fn = symbol<Connect>(so_handle, "MBXCONNX");
    return fn && h && manager && connection ? fn(h, manager, connection) : _ERROR;
}
long MBXMQ::Disc() {
    if (!con_handle) return _NORMAL;
    Close(); long rc = Disc(con_handle); delete con_handle; con_handle = nullptr; return rc;
}
long MBXMQ::Disc(MBXHMBX* h) { auto fn = symbol<Disconnect>(so_handle, "MBXDISC"); return fn && h ? fn(h) : _ERROR; }
long MBXMQ::Open(PMBXCHAR name, int mode) {
    if (!con_handle) return _ERROR;
    Close(); std::unique_ptr<MBXHMB> h(new MBXHMB{});
    long rc = Open(*con_handle, h.get(), name, mode);
    if (!rc) que_handle = h.release(); return rc;
}
long MBXMQ::Open(MBXHMBX h, MBXHMB* q, PMBXCHAR name, int mode) {
    auto fn = symbol<OpenQueue>(so_handle, "MBXOPEN"); return fn && q && name ? fn(h, q, name, mode) : _ERROR;
}
long MBXMQ::Close() {
    if (!que_handle) return _NORMAL;
    long rc = con_handle ? Close(*con_handle, que_handle) : _ERROR;
    delete que_handle; que_handle = nullptr; return rc;
}
long MBXMQ::Close(MBXHMBX h, MBXHMB* q) { auto fn = symbol<CloseQueue>(so_handle, "MBXCLOSE"); return fn && q ? fn(h, q) : _ERROR; }
long MBXMQ::Recv(PMBXCHAR name, PMBXCHAR reply, PMBXMSGID corr, PMBXCHAR data, MBXLONG cap, MBXLONG timeout) {
    if (!data || cap <= 0) return _ERROR;
    auto fn = symbol<Get>(so_handle, "MBXGET"); if (!fn) return _ERROR;
    long rc = Open(name, 0); if (rc) return rc;
    MBXMD desc{}; std::memset(data, 0, cap);
    rc = fn(*con_handle, *que_handle, nullptr, corr, data, cap, timeout, &desc);
    // Legacy reply output has no capacity parameter: caller must provide MBX_DEF_NAME_LENGTH bytes.
    if (!rc && reply) { std::memcpy(reply, desc.rmb, sizeof(desc.rmb)); reply[sizeof(desc.rmb)-1] = 0; }
    long close_rc = Close(); return rc ? rc : close_rc;
}
long MBXMQ::Recv(MBXHMBX h, MBXHMB q, CMQMsg* msg, char* id, char* corr, int timer) {
    auto fn = symbol<Get>(so_handle, "MBXGET");
    if (!fn || !msg || !msg->m_msgbuf || msg->m_msglen <= 0) return _ERROR;
    MBXMSGID a{}, b{};
    // These identifiers are fixed binary arrays, not C strings.
    if (id) std::memcpy(a, id, sizeof(a)); if (corr) std::memcpy(b, corr, sizeof(b));
    msg->m_msgdesc = MBXMD{};
    return fn(h, q, id ? &a : nullptr, corr ? &b : nullptr, msg->m_msgbuf, msg->m_msglen, timer, &msg->m_msgdesc);
}
long MBXMQ::Send(PMBXCHAR name, PMBXCHAR reply, PMBXMSGID corr, PMBXCHAR data, MBXLONG len) {
    if (!data || len < 0 || (reply && std::strlen(reply) >= sizeof(MBXCHAR128))) return _ERROR;
    auto fn = symbol<Put>(so_handle, "MBXPUT"); if (!fn) return _ERROR;
    long rc = Open(name, 1); if (rc) return rc;
    MBXMD desc{};
    if (reply) std::memcpy(desc.rmb, reply, std::strlen(reply)+1);
    if (corr) std::memcpy(desc.msgid2, *corr, sizeof(desc.msgid2));
    rc = fn(*con_handle, *que_handle, data, len, &desc);
    long close_rc = Close(); return rc ? rc : close_rc;
}
long MBXMQ::Send(MBXHMBX h, MBXHMB q, CMQMsg* msg) {
    auto fn = symbol<Put>(so_handle, "MBXPUT");
    return fn && msg && msg->m_msgbuf && msg->m_msglen >= 0 ? fn(h, q, msg->m_msgbuf, msg->m_msglen, &msg->m_msgdesc) : _ERROR;
}
