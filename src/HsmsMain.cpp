#include "HsmsSock.h"
#include "tcp_stream.h"
#include <cstdlib>
#include <exception>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
int main(int argc, char** argv) {
#ifdef _WIN32
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) return 1;
#endif
    try {
        if (argc == 3 && std::string(argv[1]) == "--listen") {
            char* end = nullptr; long port = std::strtol(argv[2], &end, 10);
            if (!end || *end || port < 1 || port > 65535) throw std::runtime_error("Port must be 1..65535");
            return run_tcp(static_cast<unsigned short>(port));
        }
        if (argc > 1) {
            std::cerr << "Usage: hsms.exe [--listen PORT]\nNo arguments: original binary stdin/stdout transport.\n";
            return std::string(argv[1]) == "--help" ? 0 : 1;
        }
        HsmsSock service; service.OnConnect(); return 0;
    } catch (const std::exception& e) { std::cerr << "HSMS error: " << e.what() << '\n'; return 1; }
}
