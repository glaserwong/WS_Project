#pragma once
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define RTLD_NOW 0
inline char*& hsms_dl_error() { static thread_local char* e = nullptr; return e; }
inline void* dlopen(const char* path, int) {
    HMODULE m = LoadLibraryA(path);
    hsms_dl_error() = m ? nullptr : const_cast<char*>("LoadLibrary failed");
    return reinterpret_cast<void*>(m);
}
inline void* dlsym(void* handle, const char* symbol) {
    FARPROC f = handle ? GetProcAddress(static_cast<HMODULE>(handle), symbol) : nullptr;
    hsms_dl_error() = f ? nullptr : const_cast<char*>("DLL handle or symbol unavailable");
    return reinterpret_cast<void*>(f);
}
inline char* dlerror() { char* e = hsms_dl_error(); hsms_dl_error() = nullptr; return e; }
inline int dlclose(void* h) { return h && FreeLibrary(static_cast<HMODULE>(h)) ? 0 : -1; }
#else
#include <dlfcn.h>
#endif
