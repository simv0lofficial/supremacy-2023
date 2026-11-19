#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/esp/weapon_icon.hpp"
#include "skeet/legit/config.hpp"

namespace skeet::ui {

class LegitWeaponSelector : public ComboBox {
public:
    LegitWeaponSelector(Widget* owner, std::int32_t* selection, WidgetIdentities& identities);
    void reset_input() override;
    bool hit_test(Point cursor) const override;
    bool mouse(const InputMessage& message, Point cursor) override;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float scale, const SliderMetrics& metrics) override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
    void on_layout() override;
    void interact_weapons(const InputMessage& message, Point cursor);
    // Explicit recovered 0x433591C2 cache boundary, shared with ESP icons.
    esp::WeaponIconProvider icons{};
    std::int32_t cell_width = 76, icon_height = 20;
};

// LEGIT / Weapon type, Aimbot, Triggerbot and Other, 0x43488883..0x43489349.
// Array bindings identify their native configuration hash; the two hotkey
// bindings use the base hash of their nine individually registered descriptors.
class LegitMenu final : public BoundMenuControls {
public:
    LegitMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities);
    ~LegitMenu();
    std::array<Category*, 4> categories() noexcept {
        return {&weapon_, aimbot_.get(), trigger_.get(), other_.get()};
    }
    LegitWeaponSelector& weapon_selector() noexcept {
        return *selector_;
    }
    std::int32_t selected_weapon() const noexcept {
        return selected_weapon_;
    }
    void append_weapon_keys(std::vector<ConfigDescriptor*>&) const;

private:
    static void weapon_changed(void* context, Widget& source);
    static void aim_key_changed(void* context, Widget& source);
    static void reaction_changed(void* context, Widget& source);
    static void accuracy_changed(void* context, Widget& source);
    void bind_weapon();
    struct ArrayBinding {
        Widget* widget;
        ConfigDescriptor* descriptor;
        bool bytes;
    };
    Tab& tab_;
    Category weapon_;
    std::unique_ptr<Category> aimbot_, trigger_, other_;
    std::int32_t selected_weapon_ = 1;
    LegitWeaponSelector* selector_ = nullptr;
    std::vector<ArrayBinding> arrays_;
    std::array<HotkeyConfig*, 9> aim_keys_{}, trigger_keys_{};
    HotkeyWidget* aim_key_ = nullptr;
    HotkeyWidget* trigger_key_ = nullptr;
    Slider* first_delay_ = nullptr;
    Slider* trigger_reaction_ = nullptr;
    Checkbox* penetration_ = nullptr;
    Checkbox* burst_ = nullptr;
    Slider* burst_time_ = nullptr;
    ComboBox* accuracy_ = nullptr;
    Slider* accuracy_range_ = nullptr;
    VisibilityLink burst_link_{};
};

} // namespace skeet::ui
