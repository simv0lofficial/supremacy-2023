#pragma once
#include "skeet/script/panorama_api.hpp"
#include <cstring>
#include <string_view>
#include <stdexcept>

namespace skeet::script::panorama_detail {
constexpr int registry = -10000, globals = -10002;
constexpr int upvalue(int index) {
    return globals - index;
}
constexpr int object_tag = 0x564f, script_tag = 0x5653;

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_PANORAMA_METHOD __thiscall
#else
#define SKEET_PANORAMA_METHOD
#endif
template <class Result, class... Args>
Result engine_call(void* object, unsigned slot, Args... args) {
    using Function = Result(SKEET_PANORAMA_METHOD*)(void*, Args...);
    auto function = reinterpret_cast<Function>((*static_cast<void***>(object))[slot]);
    return function(object, args...);
}
#undef SKEET_PANORAMA_METHOD

struct PanelRecord {
    std::int32_t left, right, parent, reserved;
    void* panel;
    std::int32_t trailing;
};
struct PanelTree {
    std::int32_t reserved;
    const PanelRecord* records;
    std::int32_t allocated, grow_size, root, count, free_list, last_allocated;
    bool valid(std::uint32_t index) const { // 0x4332B636
        return index < static_cast<std::uint32_t>(allocated) &&
               static_cast<std::int32_t>(index) <= last_allocated && index != 0xffffffffu &&
               records[index].left != static_cast<std::int32_t>(index);
    }
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(PanelRecord) == 24);
static_assert(offsetof(PanelRecord, panel) == 16);
static_assert(offsetof(PanelTree, records) == 4);
static_assert(offsetof(PanelTree, allocated) == 8);
static_assert(offsetof(PanelTree, last_allocated) == 28);
#endif
inline PanelTree panel_tree(void* engine) {
    PanelTree result{};
    std::memcpy(&result, static_cast<const std::byte*>(engine) + 0xec, sizeof(result));
    return result;
}
inline V8Local local(const PanoramaV8Api& v8, void* isolate, void** persistent) {
    return {persistent ? v8.create_handle(isolate, *persistent) : nullptr};
}
inline bool heap_string(V8Local value) {
    if (!value.handle)
        return false;
    const auto tagged = reinterpret_cast<std::uintptr_t>(*value.handle);
    if ((tagged & 3u) != 1u)
        return false;
    const auto map = *reinterpret_cast<const std::uintptr_t*>(tagged - 1);
    return *(reinterpret_cast<const std::uint8_t*>(map) + 7) < 0x80;
}
inline bool restricted_object(V8Local value) {
    if (!value.handle)
        return false;
    const auto tagged = reinterpret_cast<std::uintptr_t>(*value.handle);
    if ((tagged & 3u) != 1u)
        return false;
    const auto map = *reinterpret_cast<const std::uintptr_t*>(tagged - 1);
    if (*(reinterpret_cast<const std::uint8_t*>(map) + 7) != 0x82)
        return false;
    const auto kind = *reinterpret_cast<const std::int32_t*>(tagged + 0x17) >> 1;
    return kind == 3 || kind == 5;
}

struct Scope {
    const PanoramaV8Api& api;
    void* isolate;
    std::array<std::uintptr_t, 3> handles{};
    std::array<std::uintptr_t, 12> caught{};
    Scope(const PanoramaV8Api& imports, void* current) : api(imports), isolate(current) {
        api.isolate_enter(isolate);
        api.handle_scope_construct(handles.data(), isolate);
        api.try_catch_construct(caught.data(), isolate);
    }
    ~Scope() {
        api.try_catch_destroy(caught.data());
        api.handle_scope_destroy(handles.data());
        api.isolate_exit(isolate);
    }
    Scope(const Scope&) = delete;
    bool push_error(const LuaApi& lua, void* state, const char* substitute = nullptr,
                    bool explicit_length = true, bool known_caught = false) {
        if (!known_caught && !api.has_caught(caught.data()))
            return false;
        const auto exception = api.exception(caught.data());
        std::array<std::uintptr_t, 2> text{};
        api.utf8_construct(text.data(), exception);
        const auto* data = reinterpret_cast<const char*>(text[0]);
        const auto length = static_cast<std::uint32_t>(text[1]);
        if (!data && !substitute) {
            api.utf8_destroy(text.data());
            return false;
        }
        lua.where(state, 1);
        if (data && explicit_length)
            lua.pushlstring(state, data, length);
        else if (data)
            lua.pushstring(state, data);
        else if (substitute)
            lua.pushstring(state, substitute);
        else
            lua.pushlstring(state, "", 0);
        api.utf8_destroy(text.data());
        return true;
    }
};
struct ContextScope {
    const PanoramaV8Api& api;
    V8Local context;
    ContextScope(const PanoramaV8Api& imports, void* isolate, void*** persistent)
        : api(imports), context(local(imports, isolate, *persistent)) {
        api.context_enter(context.handle);
    }
    ~ContextScope() {
        api.context_exit(context.handle);
    }
};
inline int raise_pending(const LuaApi& api, void* state) {
    api.concat(state, 2);
    return api.error(state);
}
} // namespace skeet::script::panorama_detail
