#pragma once
#include "bridge.hpp"
#include <string_view>

namespace skeet::script {
struct VectorApi {
    int (*loadbuffer)(void*, const char*, std::size_t, const char*);
    const char* (*findtable)(void*, int, const char*, int);
};
class VectorModule {
public:
    using LoadBuffer = int (*)(void*, const char*, std::size_t, const char*);
    VectorModule(Bridge& bridge, VectorApi api, bool& loading_builtin) noexcept
        : bridge_(bridge), api_(api), loading_builtin_(loading_builtin) {}
    VectorModule(const VectorModule&) = delete;
    VectorModule& operator=(const VectorModule&) = delete;
    void preload(); // complete native 433E4D7E
    int open(); // 433879F0 -> protected loader 4333FC10
    static std::string_view source() noexcept;

private:
    Bridge& bridge_;
    VectorApi api_;
    bool& loading_builtin_;
    static int entry(void*);
    int open(void* state);
};
} // namespace skeet::script
