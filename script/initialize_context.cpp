#include "initialize_detail.hpp"
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace skeet::script {
namespace {
std::mutex owners_mutex;
struct StateOwner {
    ScriptRuntime* runtime;
    std::uintptr_t global;
    LuaVmLayout layout;
};
std::unordered_map<void*, StateOwner> owners;
}
namespace initialize_detail {
std::uintptr_t global_reference(void* state, const LuaVmLayout& layout) {
    if (!state || (layout.global_reference_size != 4 && layout.global_reference_size != 8) ||
        layout.global_reference_size > sizeof(std::uintptr_t))
        throw std::logic_error("Unsupported Lua VM global-reference layout");
    std::uintptr_t global = 0;
    std::memcpy(&global, static_cast<const std::byte*>(state) + layout.global_reference_offset,
                layout.global_reference_size);
    return global;
}
ScriptRuntime* owner(void* state) {
    if (!state)
        return nullptr;
    std::lock_guard lock(owners_mutex);
    if (const auto at = owners.find(state); at != owners.end())
        return at->second.runtime;
    // The zero-upvalue loader also runs on coroutines. Native4343704E copies
    // parent+8 into child+8, so ownership follows their shared VM reference.
    for (const auto& [main_state, entry] : owners) {
        (void)main_state;
        if (global_reference(state, entry.layout) == entry.global)
            return entry.runtime;
    }
    return nullptr;
}
void bind(ScriptRuntime& runtime) {
    const auto global = global_reference(runtime.bridge.state, runtime.api.layout);
    std::lock_guard lock(owners_mutex);
    owners.insert_or_assign(runtime.bridge.state, StateOwner{&runtime, global, runtime.api.layout});
}
void unbind(ScriptRuntime& runtime) {
    std::lock_guard lock(owners_mutex);
    for (auto at = owners.begin(); at != owners.end();) {
        if (at->second.runtime == &runtime)
            at = owners.erase(at);
        else
            ++at;
    }
}
}
ScriptRuntime::~ScriptRuntime() {
    initialize_detail::unbind(*this);
}
void release_runtime_owner(void* state) noexcept {
    std::lock_guard lock(owners_mutex);
    owners.erase(state);
}
} // namespace skeet::script
