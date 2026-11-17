#pragma once
#include "skeet/script/bridge.hpp"
#include <span>
#include <string>
#include <variant>
#include <vector>
#include <array>

namespace skeet::script {
class UiBindings;
struct MaterialRefs {
    std::array<int, 3> references{-1, -1, -1};
    void release(Bridge&); // 0x433E7E6C
};

struct BrowserState {
    explicit BrowserState(const bool& actual_unloading) noexcept
        : zero_callback_storage(actual_unloading) {}
    BrowserState(bool&&) = delete;
    BrowserState(const BrowserState&) = delete;
    BrowserState& operator=(const BrowserState&) = delete;
    // The same live 43475895 word used by material and engine teardown.
    // Its owner must outlive this state and every registered Lua closure.
    const bool& zero_callback_storage;
    bool owns_handle = false; // 0x43479604
    void** global_handle = nullptr; // 0x43479600
    std::span<const std::byte> callbacks, listeners; // 0x4347962C / 0x4347961C
    std::span<std::uint32_t> persistent_slots, callback_slots; // 0x4347963C / 0x4347960C
    // Imported v8::V8::DisposeGlobal(Object**); the embedding browser supplies
    // its own import, because the handle belongs to that V8 instance.
    void (*dispose_global)(void**) = nullptr;
    // Pointer-width equivalents of native vectors 4347963C / 4347960C and
    // their free-index stacks 4347962C / 4347961C, owned by the Panorama binder.
    std::vector<void**> object_handles, script_handles;
    std::vector<std::uint32_t> free_object_indices, free_script_indices;
    void clear(); // 0x433823FA
    // Process finalizers act on the actual Panorama handle vectors. They do
    // not select clear()'s unloading branch or touch separate raw-image spans.
    void finalize_object_handles(); //4339D4FE
    void finalize_script_handles(); //433874D2
    void finalize_global_handle(); //4341EFD3; preserves owns_handle
};

struct ReferenceString {
    int reference = -1;
    std::u16string text;
    std::uint32_t color = 0xffffffff;
};
// 0x433FC9D0 / 0x433B584A: native 24-byte reference/string records.
struct ReferenceStrings {
    std::vector<ReferenceString> entries;
    void release(Bridge&);
    void finalize_storage() noexcept; //433C956C, storage only; no Lua unref
};

// Several native extension vtables share the actual ret-4 body 0x433C95B7.
struct NoopExtension {};
using CloseExtension = std::variant<NoopExtension, UiBindings*, ReferenceStrings*, MaterialRefs*,
                                    BrowserState*, CallbackState*>;

struct ShutdownState {
    Bridge& bridge;
    // Captured end pointer in 0x4331FDC3 fixes the traversal count for this call.
    std::span<const CloseExtension> extensions;
    UiBindings* ui = nullptr;
    // Native 0x43479774 is set to begin at the end of destruction.
    std::span<const std::byte> current_config;
};

} // namespace skeet::script
