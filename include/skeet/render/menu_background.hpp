#pragma once
#include <cstdint>

namespace skeet::ui {
class MenuWindow;
class RendererBackend;

inline constexpr std::uint32_t menu_background_key = 0xd60978b0;

// Renderer context constructor,4348F8DA..4348F99B. The backend retains the
// decoded pixel bytes and owns their normal device upload/recreation.
std::uint32_t create_menu_background(RendererBackend& renderer);

// Main-window initializer43485A35..43485A49, after the tab-atlas load.
void assign_menu_background(MenuWindow& window, const RendererBackend& renderer) noexcept;
}
