#pragma once
#include "native_data_image.hpp"
#include "owner_registry.hpp"

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using GameWindowCrc32 = std::uint32_t(__stdcall*)(std::uint32_t, const void*, std::uint32_t);
using GameWindowExchange = std::int32_t(__stdcall*)(volatile std::int32_t*, std::int32_t);
#else
using GameWindowCrc32 = std::uint32_t (*)(std::uint32_t, const void*, std::uint32_t);
using GameWindowExchange = std::int32_t (*)(volatile std::int32_t*, std::int32_t);
#endif
struct GameWindowProxyImports {
    GameWindowCrc32 crc32;
    GameWindowExchange exchange;
};
// Resolves only the genuine exports in already loaded ntdll/kernel32 modules.
GameWindowProxyImports native_game_window_proxy_imports();

struct GameWindowProxyBindings {
    const std::byte* const& guarded_return_site; //43467288, verified engine caller
    void** const& game_interface_slot; //43468B2C, address of the live IGame pointer
    std::uintptr_t& original_rtti; //43469288, original vtable[-1]
    GameWindowProxyImports imports;
};
GameWindowProxyBindings native_game_window_proxy_bindings(NativeDataImage&, GameWindowProxyImports);

struct GameWindowProxyRegistries {
    OwnerRegistryBinding lifecycle; //43475B78
    OwnerRegistryBinding maps; //43475770
};
struct GameWindowProxyOwner;
std::size_t game_window_proxy_owner_size() noexcept;
// Complete4347D94F: zeroed native2C-byte prefix, lifecycle registration, map
// endpoint registration, then primary43470530 and secondary4346EF00 publication.
// Storage follows allocate_owner_storage's aligned heap-header contract.
GameWindowProxyOwner& construct_game_window_proxy_owner(void*, const GameWindowProxyRegistries&,
                                                        GameWindowProxyBindings);
// The original null allocation branch returns without registering an owner.
GameWindowProxyOwner* create_game_window_proxy_owner(const GameWindowProxyRegistries&,
                                                     GameWindowProxyBindings);

// Complete primary1, map0 and primary4. Initialization scans four-byte CRC
// windows until B94D4C7E; its native CMP200 is followed by an unconditional jump
// and does not impose a search bound. The verified loaded-build guard must have
// a readable match. The proxy preserves all25 native virtual tail transfers;
// slot9 restores the real interface and changes only the matched caller return.
void initialize_game_window_proxy(GameWindowProxyOwner&);
void install_game_window_proxy(GameWindowProxyOwner&);
void restore_game_window_proxy(GameWindowProxyOwner&);
// Perform primary4 before lifecycle destruction while a proxy can still be used.
} // namespace skeet::engine
