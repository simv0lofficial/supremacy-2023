#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/anti_aim/menu_dependencies.hpp"

namespace skeet::ui {
// AA / Anti-aimbot angles, 0x43487FF7..0x43488595. Settings and the supplied
// tab must outlive this group. The five dependent controls begin hidden.
class AntiAimMenu final : public BoundMenuControls {
public:
    AntiAimMenu(Tab&, ConfigRegistries&, WidgetIdentities&);
    ~AntiAimMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    anti_aim::MenuDependencies dependencies();
    static void pitch_changed(void*, Widget&);
    static void yaw_changed(void*, Widget&);
    static void jitter_changed(void*, Widget&);
    static void body_changed(void*, Widget&);
    Tab& tab_;
    Category category_;
    ComboBox* yaw_ = nullptr;
    ComboBox* jitter_ = nullptr;
    Slider* pitch_offset_ = nullptr;
    Slider* yaw_offset_ = nullptr;
    Slider* jitter_offset_ = nullptr;
    Slider* body_offset_ = nullptr;
    Checkbox* freestanding_body_ = nullptr;
    std::int32_t* yaw_offsets_ = nullptr;
    std::int32_t* body_offsets_ = nullptr;
};
} // namespace skeet::ui
