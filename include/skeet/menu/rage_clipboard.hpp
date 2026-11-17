#pragma once
#include "skeet/menu/rage_weapon_groups.hpp"
#include "skeet/menu/rage_weapon_selector.hpp"

namespace skeet::ui {
struct RageClipboardApi {
    void* context;
    bool (*open)(void*, std::uintptr_t owner);
    bool (*empty)(void*);
    std::uintptr_t (*allocate)(void*, std::uint32_t flags, std::size_t size);
    std::uintptr_t (*set_data)(void*, std::uint32_t format, std::uintptr_t memory);
    void (*free)(void*, std::uintptr_t memory);
    void (*close)(void*);
    bool (*available)(void*, std::uint32_t format);
    std::uintptr_t (*get_data)(void*, std::uint32_t format);
    const char* (*wire)(void*, std::uintptr_t memory);
    std::size_t (*size)(void*, std::uintptr_t memory);
    void (*unlock)(void*, std::uintptr_t memory);
};
RageClipboardApi& system_rage_clipboard() noexcept;

void write_rage_clipboard(std::span<const std::byte> value,
                          RageClipboardApi& api = system_rage_clipboard());
// Failures before GlobalWire succeeds preserve the destination. An empty
// or undecodable string replaces it with the decoder's empty result.
void read_rage_clipboard(ConfigBytes& destination, RageClipboardApi& api = system_rage_clipboard());

struct RageWeaponClipboardContext {
    RageWeaponGroupState& state;
    const std::uint32_t* selected_group;
    RageClipboardApi* clipboard = &system_rage_clipboard();
};
RageWeaponSelectorActions
rage_weapon_clipboard_actions(RageWeaponClipboardContext& context) noexcept;

struct RageWeaponEnabledSettings {
    static constexpr std::array<std::uint32_t, 12> hashes{
        0xaa63a742, 0xfdb34d7a, 0x620db85c, 0xdc62c451, 0x4841ebb5, 0x0c71a448,
        0xd981aa17, 0xf6840d17, 0xe6e1718d, 0x0fd2e90b, 0x50b604ba, 0x9cf9d46e};
    // Original static metadata precedes the individual43492C89..43492DFF
    // callbacks. Those callbacks append links and write only the value byte.
    std::array<BooleanConfig, 12> values{
        {{{{nullptr, nullptr, 0xaa63a742}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0xfdb34d7a}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0x620db85c}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0xdc62c451}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0x4841ebb5}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0x0c71a448}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0xd981aa17}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0xf6840d17}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0xe6e1718d}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0x0fd2e90b}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0x50b604ba}, 1, ConfigType::boolean, 0}, 0},
         {{{nullptr, nullptr, 0x9cf9d46e}, 1, ConfigType::boolean, 0}, 0}}};
    RageWeaponEnabledSettings() = default;
    RageWeaponEnabledSettings(const RageWeaponEnabledSettings&) = delete;
    RageWeaponEnabledSettings& operator=(const RageWeaponEnabledSettings&) = delete;
    // Explicit grouped registration utility; native startup instead binds
    // the twelve individual callbacks through rage::GroupInitializerBindings.
    void register_config(ConfigLink& registry) noexcept;
    std::array<BooleanConfig*, 12> descriptors() noexcept;
    void bind(RageWeaponSelector& selector) noexcept;
};
}
