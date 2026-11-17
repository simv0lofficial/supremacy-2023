#pragma once
#include "skeet/script/bridge.hpp"

namespace skeet::script {
struct CjsonRegistration {
    const char* name;
    LuaApi::CFunction function;
};
// Genuine Lua 5.1 C API entries. The client JSON implementation is source in
// cjson_*.cpp; none of these pointers substitutes for a client JSON helper.
struct CjsonApi {
    LuaApi lua;
    const char* (*type_name)(void*, int) = nullptr;
    double (*to_number)(void*, int) = nullptr;
    void* (*new_userdata)(void*, std::size_t) = nullptr;
    int (*set_metatable)(void*, int) = nullptr;
    int (*check_stack)(void*, int) = nullptr;
    void (*push_boolean)(void*, int) = nullptr;
    int (*check_option)(void*, int, const char*, const char* const*) = nullptr;
    void (*open_library)(void*, const char*, const CjsonRegistration*, int) = nullptr;
};

// Native 433C8D0F exports the modified Lua-CJSON core under global "json".
// Its eight methods share a garbage-collected configuration userdata. This
// variant removes keep_buffer/new/safe and renames encode/decode to
// stringify/parse. Upstream 2.0.0 and 2.1.0 have an identical conversion core;
// the stripped client has no evidence that uniquely identifies either tag.
class JsonModule {
public:
    JsonModule(Bridge& bridge, CjsonApi api) : bridge_(bridge), api_(api) {}
    int register_exports(); // leaves and returns one module table
private:
    Bridge& bridge_;
    CjsonApi api_;
};
} // namespace skeet::script
