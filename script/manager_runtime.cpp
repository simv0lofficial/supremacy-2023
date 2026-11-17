#include "skeet/script/manager.hpp"
#include "manager_detail.hpp"
#include "detail.hpp"

namespace skeet::script {
using namespace detail;
int ScriptManager::load_module(std::string_view module) { //433BFE23
    const std::lock_guard lock(bridge_.vm_mutex);
    if (bridge_.runtime_ref == -1)
        return 1;
    const auto& api = *bridge_.api;
    auto* state = bridge_.state;
    CallbackFrame frame{};
    frame.active.flags = 0x67736C6A;
    frame.saved_top = api.gettop(state);
    const std::string name(module);
    RtlPushFrame(&frame.active);
    int result = 1;
    auto invoke = [&] {
        api.rawgeti(state, -10000, bridge_.runtime_ref);
        api.rawgeti(state, -1, 3);
        api.remove(state, -2);
        api.pushstring(state, name.c_str());
        result = callback_pcall(state, 1, 0);
        if (result)
            report_lua_error(bridge_);
    };
    const auto completed = protected_region(invoke, frame);
    api.settop(state, frame.saved_top);
    RtlPopFrame(&frame.active);
    if (!completed) {
        report_native_exception(bridge_, frame, name.c_str());
        return 1;
    }
    return result;
}
void ScriptManager::initialize_modules(std::set<std::u16string>& modules) { //433B3378
    current_ui_config_.clear();
    shutdown_.current_config = current_ui_config_;
    if (modules.empty() && builtins_.empty())
        return;
    auto* state = runtime_.ensure_state();
    if (!state)
        return;
    const auto& api = *bridge_.api;
    if (!modules.empty()) {
        api.gc(state, 0, 0);
        for (auto it = modules.begin(); it != modules.end();) {
            if (load_module(manager_detail::utf8(*it)) == 0)
                ++it;
            else
                it = modules.erase(it);
        }
        api.gc(state, 1, -1);
        api.gc(state, 2, 0);
    }
    if (modules.empty() && builtins_.empty()) {
        notify_shutdown(bridge_);
        destroy_runtime(shutdown_);
    }
    if (ready()) {
        current_ui_config_ = save_ui_values();
        shutdown_.current_config = current_ui_config_;
        bridge_.api->gc(bridge_.state, 2, 0);
    }
}
bool ScriptManager::load_name(std::u16string_view name) { //43419B27
    const std::lock_guard lock(bridge_.vm_mutex);
    if (const auto nul = name.find(u'\0'); nul != name.npos)
        name = name.substr(0, nul);
    if (!name.empty() && name.front() == u'*') {
        const auto hash = script_name_hash(name);
        auto found = builtins_.end();
        for (auto it = builtins_.begin(); it != builtins_.end(); ++it)
            if (!it->enabled && script_name_hash(std::string_view(it->name)) == hash) {
                found = it;
                break;
            }
        if (found == builtins_.end())
            return false;
        found->enabled = true;
        notify_shutdown(bridge_);
        const auto saved = save_ui_values();
        destroy_runtime(shutdown_);
        initialize_modules(loaded_files_);
        bool result = false;
        if (!ready()) {
            found->enabled = false;
            initialize_modules(loaded_files_);
        } else
            result = found->enabled;
        if (ready())
            restore_ui_values(saved);
        return result;
    }
    auto candidate = loaded_files_;
    candidate.emplace(name);
    notify_shutdown(bridge_);
    const auto saved = save_ui_values();
    destroy_runtime(shutdown_);
    initialize_modules(candidate);
    if (!ready()) {
        if (!loaded_files_.empty()) {
            initialize_modules(loaded_files_);
            restore_ui_values(saved);
        }
    } else {
        loaded_files_ = std::move(candidate);
        restore_ui_values(saved);
    }
    return loaded_files_.contains(std::u16string(name));
}
bool ScriptManager::unload_name(std::u16string_view name) { //433DF341
    const std::lock_guard lock(bridge_.vm_mutex);
    if (const auto nul = name.find(u'\0'); nul != name.npos)
        name = name.substr(0, nul);
    if (!name.empty() && name.front() == u'*') {
        const auto hash = script_name_hash(name);
        auto found = builtins_.end();
        for (auto it = builtins_.begin(); it != builtins_.end(); ++it)
            if (it->enabled && script_name_hash(std::string_view(it->name)) == hash) {
                found = it;
                break;
            }
        if (found == builtins_.end())
            return false;
        found->enabled = false;
    } else {
        auto found = loaded_files_.find(std::u16string(name));
        if (found == loaded_files_.end())
            return false;
        loaded_files_.erase(found);
    }
    initialize();
    return true;
}
void ScriptManager::update(std::uint8_t& reload_requested) { //4338174B/433882DB
    const std::lock_guard lock(bridge_.vm_mutex);
    if (!ready())
        return;
    if (reload_requested) {
        reload_requested = 0;
        initialize();
        return;
    }
    if (bridge_.runtime_ref == -1)
        return;
    const auto& api = *bridge_.api;
    auto* state = bridge_.state;
    CallbackFrame frame{};
    frame.active.flags = 0x67736C6A;
    frame.saved_top = api.gettop(state);
    RtlPushFrame(&frame.active);
    auto invoke = [&] {
        api.rawgeti(state, -10000, bridge_.runtime_ref);
        api.rawgeti(state, -1, 6);
        api.remove(state, -2);
        if (api.pcall(state, 0, 0, 0))
            report_lua_error(bridge_);
    };
    protected_region(invoke, frame);
    api.settop(state, frame.saved_top);
    RtlPopFrame(&frame.active);
}
} // namespace skeet::script
