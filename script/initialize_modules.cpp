#include "skeet/script/initialize.hpp"
#include "initialize_detail.hpp"
#include "skeet/script/cjson.hpp"
#include "skeet/script/vector.hpp"
#include "skeet/script/compat.hpp"
#include "skeet/script/registry.hpp"
#include <cstring>
#include <stdexcept>

namespace skeet::script {
void ScriptRuntime::register_modules() { // 433EC568
    if (!json_module || !vector_module || !extensions || !compat_module || !api.openlibs)
        throw std::logic_error("Every native script module initializer must be bound");
    const auto& lua = *bridge.api;
    auto* state = bridge.state;
    lua.gettop(state);
    api.openlibs(state);
    const auto global = initialize_detail::global_reference(state, api.layout);
    const std::uint32_t limit = (executable_block_count & 0xffffu) << 6;
    std::memcpy(reinterpret_cast<std::byte*>(global) + api.layout.machine_code_limit_offset, &limit,
                sizeof(limit));
    json_module->register_exports();
    lua.pushvalue(state, -1);
    lua.setfield(state, -10002, "json");
    vector_module->preload();
    extensions->register_exports(bridge);
    if (compat_module->open() == 0)
        bridge.runtime_ref = lua.ref(state, -10000);
    else
        bridge.runtime_ref = -1;
    install_loaders();
    run_builtins();
    lua.gettop(state);
}
} // namespace skeet::script
