#pragma once
#include "skeet/config_storage/storage.hpp"
#include "skeet/menu_window.hpp"
#include <string_view>

namespace skeet::ui {
std::uint32_t category_name_hash(std::string_view name) noexcept;
std::uint32_t category_name_hash(std::u16string_view name) noexcept;
Tab* find_config_tab(MenuWindow& window, std::uint32_t hash);
Category* find_config_category(Tab& tab, std::uint32_t hash);
std::vector<ConfigDescriptor*> collect_category_config(Category& category, ConfigLink& registry);
bool category_config_loading() noexcept;
// The native loading byte is one during all three passes. Hotkeys preserve
// their current toggle bit while the baseline and selected values are copied.
void load_category_config_values(std::span<ConfigDescriptor*> descriptors,
                                 std::span<const std::byte> selected,
                                 std::span<const std::byte> baseline_a = {},
                                 std::span<const std::byte> baseline_b = {});
bool save_category_config(MenuWindow& window, ConfigLink& registry, std::string_view tab,
                          std::string_view category, ConfigBytes& output,
                          std::span<const std::byte> baseline_a = {},
                          std::span<const std::byte> baseline_b = {});
bool load_category_config(MenuWindow& window, ConfigLink& registry, std::string_view tab,
                          std::string_view category, std::span<const std::byte> selected,
                          std::span<const std::byte> baseline_a = {},
                          std::span<const std::byte> baseline_b = {});
}
