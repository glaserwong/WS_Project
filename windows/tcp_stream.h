#pragma once
#include <streambuf>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using NativeSocket = SOCKET;
constexpr NativeSocket BadSocket = INVALID_SOCKET;
inline void close_socket(NativeSocket s) { closesocket(s); }
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using NativeSocket = int;
constexpr NativeSocket BadSocket = -1;
inline void close_socket(NativeSocket s) { close(s); }
#endif
class TcpRuntime {
public:
    TcpRuntime() {
#ifdef _WIN32
        WSADATA data{}; if (WSAStartup(MAKEWORD(2,2), &data)) throw std::runtime_error("WSAStartup failed");
#endif
    }
    ~TcpRuntime() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};
struct SocketOwner {
    NativeSocket s = BadSocket;
    explicit SocketOwner(NativeSocket value) : s(value) {}
    ~SocketOwner() { if (s != BadSocket) close_socket(s); }
    SocketOwner(const SocketOwner&) = delete;
    SocketOwner& operator=(const SocketOwner&) = delete;
};
class SocketStream : public std::streambuf {
    NativeSocket s_; char byte_;
protected:
    int_type underflow() override {
        if (gptr() && gptr() < egptr()) return traits_type::to_int_type(*gptr());
        if (recv(s_, &byte_, 1, 0) != 1) return traits_type::eof();
        setg(&byte_, &byte_, &byte_+1); return traits_type::to_int_type(byte_);
    }
    std::streamsize xsgetn(char* p, std::streamsize n) override {
        std::streamsize done = 0;
        if (n > 0 && gptr() && gptr() < egptr()) { *p = *gptr(); gbump(1); ++done; }
        while (done < n) {
            int amount = static_cast<int>(std::min<std::streamsize>(n-done, 65536));
            int got = static_cast<int>(recv(s_, p+done, amount, 0));
            if (got <= 0) break;
            done += got;
        }
        return done;
    }
    std::streamsize xsputn(const char* p, std::streamsize n) override {
        std::streamsize done = 0;
        while (done < n) {
            int amount = static_cast<int>(std::min<std::streamsize>(n-done, 65536));
#ifdef __APPLE__
            int flags = 0;
#else
#ifdef _WIN32
            int flags = 0;
#else
            int flags = MSG_NOSIGNAL;
#endif
#endif
            int sent = static_cast<int>(send(s_, p+done, amount, flags));
            if (sent <= 0) break; done += sent;
        }
        return done;
    }
    int_type overflow(int_type c) override {
        if (traits_type::eq_int_type(c, traits_type::eof())) return traits_type::not_eof(c);
        char b = traits_type::to_char_type(c); return xsputn(&b, 1) == 1 ? c : traits_type::eof();
    }
public:
    explicit SocketStream(NativeSocket s) : s_(s) {}
};
class RedirectStreams {
    std::streambuf *in_, *out_;
public:
    explicit RedirectStreams(std::streambuf* b) : in_(std::cin.rdbuf(b)), out_(std::cout.rdbuf(b)) {}
    ~RedirectStreams() { std::cin.rdbuf(in_); std::cout.rdbuf(out_); }
};
inline int run_tcp(unsigned short port) {
    TcpRuntime runtime;
    SocketOwner listener(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listener.s == BadSocket) throw std::runtime_error("socket failed");
    sockaddr_in address{}; address.sin_family = AF_INET; address.sin_addr.s_addr = htonl(INADDR_ANY); address.sin_port = htons(port);
    if (bind(listener.s, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(listener.s, 1) != 0) throw std::runtime_error("TCP bind/listen failed");
    std::cerr << "HSMS listening on port " << port << " (one connection per invocation)\n";
    SocketOwner client(accept(listener.s, nullptr, nullptr));
    if (client.s == BadSocket) throw std::runtime_error("accept failed");
#ifdef __APPLE__
    int no_sigpipe = 1; setsockopt(client.s, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe));
#endif
    SocketStream stream(client.s); RedirectStreams redirect(&stream);
    HsmsSock service; service.OnConnect(); return 0;
}
