#include "skeet/script/shutdown.hpp"
#include "skeet/script/ui.hpp"
#include "skeet/script/initialize.hpp"
#include "detail.hpp"
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace skeet::script {
using namespace detail;
namespace {
void release_browser_handles(std::vector<void**>& handles, void (*dispose)(void**)) {
    auto* cursor = handles.data();
    auto* const end = handles.empty() ? cursor : cursor + handles.size();
    for (; cursor != end; ++cursor) {
        if (!*cursor)
            continue;
        if (!dispose)
            throw std::logic_error("Owned V8 slot requires DisposeGlobal");
        dispose(*cursor);
        *cursor = nullptr;
    }
    // Native frees the current begin after traversing its captured range.
    std::vector<void**>{}.swap(handles);
}
}

void ReferenceStrings::release(Bridge& bridge) {
    for (auto& entry : entries)
        release_reference(*bridge.api, bridge.state, entry.reference);
    entries.clear();
}
void ReferenceStrings::finalize_storage() noexcept {
    // The terminal helper destroys text storage in forward record order. Lua
    //references were released by the separate runtime-close extension.
    for (auto& entry : entries)
        std::u16string{}.swap(entry.text);
    std::vector<ReferenceString>{}.swap(entries);
}

void MaterialRefs::release(Bridge& bridge) {
    const auto& api = *bridge.api;
    for (auto& reference : references)
        release_reference(api, bridge.state, reference);
    api.getfield(bridge.state, -10002, "materialsystem");
    api.rawgeti(bridge.state, -1, 1);
    if (api.type(bridge.state, -1) == 5) {
        // 0x43431CD2 empties the array and hash parts of the table without
        // replacing the object or its metatable. Use the embedding Lua API.
        api.pushnil(bridge.state);
        while (api.next(bridge.state, -2)) {
            api.settop(bridge.state, -2);
            api.pushvalue(bridge.state, -1);
            api.pushnil(bridge.state);
            api.rawset(bridge.state, -4);
        }
        api.settop(bridge.state, -3);
    }
}

void BrowserState::clear() {
    callbacks = callbacks.first(0);
    listeners = listeners.first(0);
    free_object_indices.clear();
    free_script_indices.clear();
    if (zero_callback_storage) {
        if (owns_handle)
            global_handle = nullptr;
        std::fill(persistent_slots.begin(), persistent_slots.end(), 0);
        std::fill(callback_slots.begin(), callback_slots.end(), 0);
        std::fill(object_handles.begin(), object_handles.end(), nullptr);
        std::fill(script_handles.begin(), script_handles.end(), nullptr);
    } else if (owns_handle) {
        if (global_handle) {
            if (!dispose_global)
                throw std::logic_error("Owned V8 handle requires DisposeGlobal");
            dispose_global(global_handle);
            global_handle = nullptr;
        }
        owns_handle = false;
    }
}
void BrowserState::finalize_object_handles() {
    release_browser_handles(object_handles, dispose_global);
}
void BrowserState::finalize_script_handles() {
    release_browser_handles(script_handles, dispose_global);
}
void BrowserState::finalize_global_handle() {
    if (!owns_handle || !global_handle)
        return;
    if (!dispose_global)
        throw std::logic_error("Owned V8 handle requires DisposeGlobal");
    dispose_global(global_handle);
    global_handle = nullptr;
}

void destroy_runtime(ShutdownState& runtime) {
    auto& bridge = runtime.bridge;
    const std::lock_guard lock(bridge.vm_mutex);
    if (bridge.state) {
        const auto& api = *bridge.api;
        void* const state = bridge.state;
        CallbackFrame guard{};
        guard.active.flags = 0x67736c6a;
        guard.saved_top = api.gettop(state);
        RtlPushFrame(&guard.active);
        auto invoke = [&] {
            const auto extensions = runtime.extensions;
            for (const auto& extension : extensions) {
                std::visit(
                    [&](const auto& current) {
                        using Type = std::decay_t<decltype(current)>;
                        if constexpr (std::is_same_v<Type, NoopExtension>) {
                            return;
                        } else if constexpr (std::is_same_v<Type, UiBindings*>) {
                            current->release_callbacks(bridge);
                        } else if constexpr (std::is_same_v<Type, BrowserState*>) {
                            current->clear();
                        } else {
                            current->release(bridge);
                        }
                    },
                    extension);
            }
            if (bridge.runtime_ref != -1) {
                api.rawgeti(state, -10000, bridge.runtime_ref);
                api.rawgeti(state, -1, 7);
                api.remove(state, -2);
                if (api.pcall(state, 0, 0, 0) != 0)
                    report_lua_error(bridge);
            }
            (void)api.gc(state, 2, 0);
            release_reference(api, state, bridge.runtime_ref);
            (void)api.gc(state, 2, 0);
        };
        protected_region(invoke, guard);
        api.settop(state, guard.saved_top);
        RtlPopFrame(&guard.active);
        api.close(state);
        release_runtime_owner(state);
        bridge.state = nullptr;
    }
    if (runtime.ui)
        runtime.ui->destroy_bindings();
    runtime.current_config = runtime.current_config.first(0);
}

} // namespace skeet::script
