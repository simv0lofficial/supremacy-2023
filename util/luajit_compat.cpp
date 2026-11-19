#include <cstdio>
#include <cstdint>
#include <cstdarg>

extern "C" {

// 64-bit integer arithmetic builtins for 32-bit x86
int64_t __cdecl __divdi3(int64_t a, int64_t b) {
    return a / b;
}

uint64_t __cdecl __udivdi3(uint64_t a, uint64_t b) {
    return a / b;
}

int64_t __cdecl __moddi3(int64_t a, int64_t b) {
    return a % b;
}

uint64_t __cdecl __umoddi3(uint64_t a, uint64_t b) {
    return a % b;
}

int compat_fscanf(FILE* stream, const char* format, ...) __asm__("_fscanf");
int compat_fscanf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfscanf(stream, format, args);
    va_end(args);
    return ret;
}

// 64-bit file seek/tell wrappers
int __cdecl compat_fseeko64(FILE* stream, int64_t offset, int origin) {
    return _fseeki64(stream, offset, origin);
}

int64_t __cdecl compat_ftello64(FILE* stream) {
    return _ftelli64(stream);
}

decltype(&compat_fseeko64) imp_fseeko64 __asm__("__imp__fseeko64") = &compat_fseeko64;
decltype(&compat_ftello64) imp_ftello64 __asm__("__imp__ftello64") = &compat_ftello64;

int lj_err_unwind_win(void* rec, void* f, void* ctx, void* disp);

uintptr_t skeet_lua_seh_handler_address(void) {
    return (uintptr_t)&lj_err_unwind_win;
}

int skeet_lua_can_unwind_seh_frame(const void* registration) {
    return registration != nullptr;
}

}
