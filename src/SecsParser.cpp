#include "SecsParser.h"
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <limits>
namespace {
uint32_t be32(const unsigned char* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
}
std::string escape(const std::string& s) {
    std::string r;
    for (unsigned char c : s) {
        switch (c) {
        case '&': r += "&amp;"; break; case '<': r += "&lt;"; break;
        case '>': r += "&gt;"; break; case '"': r += "&quot;"; break;
        case '\'': r += "&apos;"; break;
        default:
            if (c < 32 && c != 9 && c != 10 && c != 13) throw std::runtime_error("Invalid XML character");
            if (c >= 128) throw std::runtime_error("Non-ASCII text requires an explicit encoding policy");
            r += static_cast<char>(c);
        }
    }
    return r;
}
class Decoder {
    const unsigned char* p_; size_t n_, pos_ = 14;
    void need(size_t n) { if (n > n_ - pos_) throw std::runtime_error("Truncated SECS item"); }
public:
    Decoder(const unsigned char* p, size_t n) : p_(p), n_(n) {}
    std::string item(unsigned depth, unsigned index) {
        if (depth > 64) throw std::runtime_error("SECS nesting limit exceeded");
        need(1); unsigned head = p_[pos_++], bytes = head & 3, type = head & 0xfc;
        if (!bytes) throw std::runtime_error("Invalid SECS length-byte count");
        need(bytes); uint32_t len = 0;
        while (bytes--) len = (len << 8) | p_[pos_++];
        std::ostringstream out, value;
        const std::string tag = "LV_" + std::to_string(depth) + "_IT_" + std::to_string(index);
        if (type == 0) {
            // Each child needs at least a format byte and one length byte.
            if (len > (n_ - pos_) / 2) throw std::runtime_error("Impossible SECS list count");
            out << '<' << tag << " T=\"L\" L=\"1\" value=\"" << len << "\">";
            for (uint32_t i = 1; i <= len; ++i) out << item(depth + 1, i);
            out << "</" << tag << '>'; return out.str();
        }
        need(len); const unsigned char* data = p_ + pos_; pos_ += len;
        const char* name = nullptr; unsigned width = 1;
        switch (type) {
        case 0x20: name = "B"; break; case 0x24: name = "T"; break;
        case 0x40: name = "A"; break; case 0x44: name = "J"; break;
        case 0x60: name = "I8"; width = 8; break; case 0x64: name = "I1"; break;
        case 0x68: name = "I2"; width = 2; break; case 0x70: name = "I4"; width = 4; break;
        case 0x80: name = "F8"; width = 8; break; case 0x90: name = "F4"; width = 4; break;
        case 0xa0: name = "U8"; width = 8; break; case 0xa4: name = "U1"; break;
        case 0xa8: name = "U2"; width = 2; break; case 0xb0: name = "U4"; width = 4; break;
        default: throw std::runtime_error("Unknown SECS item format");
        }
        if (len % width) throw std::runtime_error("Misaligned SECS numeric length");
        if (type == 0x40 || type == 0x44) value << escape(std::string(reinterpret_cast<const char*>(data), len));
        else for (uint32_t i = 0; i < len; i += width) {
            uint64_t u = 0; for (unsigned j = 0; j < width; ++j) u = (u << 8) | data[i+j];
            if (type == 0x24) value << (u ? "T" : "F");
            else if (type == 0x90) { uint32_t bits = static_cast<uint32_t>(u); float f; std::memcpy(&f, &bits, 4); value << std::setprecision(9) << f; }
            else if (type == 0x80) { double d; std::memcpy(&d, &u, 8); value << std::setprecision(17) << d; }
            else if (type >= 0x60 && type <= 0x70) {
                if ((u >> (width*8 - 1)) != 0) {
                    const uint64_t magnitude = width == 8 ? (~u + 1) : ((uint64_t(1) << (width*8)) - u);
                    value << '-' << magnitude;
                } else value << u;
            } else value << u;
            value << ' ';
        }
        out << '<' << tag << " T=\"" << name << "\" L=\"" << len/width << "\" value=\"" << value.str() << "\" />";
        return out.str();
    }
    bool finished() const { return pos_ == n_; }
};
}
SecsParser::SecsParser() { InitAttribute(); }
SecsParser::SecsParser(char* data, int length) { InitAttribute(); SetSECS(data, length); }
void SecsParser::SetSECS(char* data, int length) {
    InitAttribute();
    if (!data || length < 14) throw std::runtime_error("HSMS header is truncated");
    const auto* p = reinterpret_cast<const unsigned char*>(data);
    if (be32(p) != static_cast<uint32_t>(length - 4)) throw std::runtime_error("HSMS frame length mismatch");
    const uint32_t sys = be32(p+10);
    // Preserve the historical long API; XML always carries all 32 unsigned bits.
    int32_t signed_sys; std::memcpy(&signed_sys, &sys, 4);
    system_bytes = signed_sys; funnum = p[7];
    std::ostringstream out;
    out << "<SECS Stream = \"S" << (p[6] & 0x7f) << 'F' << unsigned(p[7]) << "\" systembyte = \"" << sys << "\" >";
    if (length > 14) {
        Decoder decoder(p, static_cast<size_t>(length));
        out << decoder.item(1, 1);
        if (!decoder.finished()) throw std::runtime_error("Trailing SECS body data");
    }
    out << "</SECS>"; secs_xml = out.str();
}
std::string SecsParser::GetXML() { return secs_xml; }
long SecsParser::GetFunNum() { return funnum; }
long SecsParser::GetSysByte() { return system_bytes; }
void SecsParser::InitAttribute() { secs_xml.clear(); system_bytes = 0; funnum = 0; }
