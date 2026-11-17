#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace skeet::ui {
// The main-window producer owns the native32-code-unit inline registry name.
// The source controller has a different layout, so CONFIG borrows this typed
// storage instead of interpreting RootMenu+E0 as a native window address.
// Keep this owner alive through every settings snapshot that captures it.
struct MainMenuRegistryName {
    std::array<char16_t, 32> text{};
    std::u16string_view view() const;
};
struct MainMenuCpuLeaf1 {
    std::uint32_t eax, ebx, ecx, edx;
};
constexpr std::uint32_t main_menu_registry_value(MainMenuCpuLeaf1 cpu) noexcept {
    return std::uint32_t((cpu.eax ^ cpu.ecx ^ cpu.edx) + 4u) >> 1;
}
MainMenuCpuLeaf1 read_main_menu_cpu_leaf1();
MainMenuRegistryName make_main_menu_registry_name(MainMenuCpuLeaf1) noexcept;
MainMenuRegistryName make_main_menu_registry_name();
} //namespace skeet::ui
