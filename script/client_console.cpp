#include "client_detail.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
ConsoleInputHook::ConsoleInputHook(engine::HookDescriptorStorage& storage, Bridge& runtime)
    : storage(storage), metadata(storage.at(0x434758b8)), bridge(runtime) {}
void ConsoleInputHook::capture(void* engine_container, void* callback_owner) { //43482378
    if (!engine_container)
        return;
    auto* object = client_detail::field<void*>(engine_container, 8);
    if (!object)
        return;
    auto* code = static_cast<const std::byte*>((*static_cast<void***>(object))[279]);
    std::uint32_t prefix;
    std::memcpy(&prefix, code, sizeof(prefix));
    if (prefix != 0x8bec8b55u || code[4] != std::byte{0x0d})
        return;
    metadata.target = object;
    metadata.owner = callback_owner;
}
void ConsoleInputHook::enable() const { //433E2344
    if (metadata.replacement_table)
        InterlockedExchangePointer(static_cast<void* volatile*>(metadata.target),
                                   metadata.replacement_table);
}
void ConsoleInputHook::restore() const { //433F8C58
    if (metadata.replacement_table && metadata.target)
        InterlockedExchangePointer(static_cast<void* volatile*>(metadata.target),
                                   metadata.original_table);
}
void ConsoleInputHook::process(void* object, const char* text) const { //433410B6
    const auto table = *static_cast<void***>(object);
    const auto* descriptor = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    using Original = engine::MemberFunction<void, const char*>;
    const auto original = reinterpret_cast<Original>(descriptor->original_table[279]);
    // The native snapshots the original entry before any Lua callback runs.
    if (text && *text && bridge.state && dispatch_console_input(bridge, text))
        return;
    original(object, text);
}
void ConsoleInputHook::prepare(engine::NativeImageEnvironment images, engine::JobMemory memory,
                               engine::HookDescriptorSystem system) {
    auto* const client_module = storage.image().native_cell<std::byte*>(0x4346a55c);
    engine::prepare_rtti_vtable_hook(metadata, client_module, 0x46e07364, 0, images, memory,
                                     system);
    metadata.replacement_table[279] = reinterpret_cast<void*>(&console_input_hook);
}
void ConsoleInputHook::clear(engine::JobMemory memory) {
    engine::clear_vtable_hook(metadata, memory);
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall console_input_hook(void* object, void*, const char* text) { //4334E722
#else
void console_input_hook(void* object, void*, const char* text) {
#endif
    const auto table = *static_cast<void***>(object);
    const auto* descriptor = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    auto* owner = descriptor->owner;
    const auto owner_table = *static_cast<void***>(owner);
    using OwnerEntry = engine::MemberFunction<void, void*, const char*>;
    reinterpret_cast<OwnerEntry>(owner_table[9])(owner, object, text);
}
} // namespace skeet::script
