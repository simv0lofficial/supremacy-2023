#pragma once
#include "skeet/script/cjson.hpp"
#include "skeet/script/initialize.hpp"
#include "skeet/script/vector.hpp"

namespace skeet::script {

// One directly linked x86/FR1 LuaJIT runtime backs every script API view.
// These bindings own no state; ScriptRuntime retains its normal VM lifetime.
struct LuaRuntimeBindings {
    LuaApi lua;
    InitializationApi initialization;
    CjsonApi json;
    VectorApi vector;
};

const LuaRuntimeBindings& source_lua_runtime_bindings() noexcept;

} // namespace skeet::script
