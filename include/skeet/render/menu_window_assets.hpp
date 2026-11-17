#pragma once
#include <cstdint>

namespace skeet::ui {
class MenuWindow;
class RendererBackend;

struct MainMenuAssets {
    std::uint32_t tab_atlas = UINT32_MAX; // native mainwindow+D8
};

//434801C9's resource25D9 is the provided decoded384x48 RGBA atlas.
std::uint32_t create_menu_tab_atlas(RendererBackend& renderer);

// Complete mainwindow vslot+4,43485A05. Composition must already own its tabs.
void initialize_main_menu_assets(MenuWindow& window, RendererBackend& renderer,
                                 MainMenuAssets& assets);
}
