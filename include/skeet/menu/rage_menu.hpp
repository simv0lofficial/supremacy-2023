#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/menu/rage_weapon_selector.hpp"

namespace skeet::ui {
// 0x43480054 / 0x4333C703. Values above 100 render as HP+N.
class RageDamageSlider final : public Slider {
public:
    RageDamageSlider(Widget* owner, std::u16string_view label, std::int32_t* setting,
                     WidgetIdentities& identities);
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
};

// RAGE / Weapon type, 0x434870BE..0x4348713B. The selector's value is
// the RAGE tab's selection field, not a configuration descriptor.
class RageWeaponMenu final : public BoundMenuControls {
public:
    RageWeaponMenu(Tab& tab, std::uint32_t& selected_group, std::span<std::uint8_t> enabled_groups,
                   RageWeaponSelectorActions actions, WidgetIdentities& identities);
    RageWeaponMenu(Tab& tab, std::uint32_t& selected_group,
                   std::span<BooleanConfig*, 12> enabled_groups, RageWeaponSelectorActions actions,
                   WidgetIdentities& identities);
    ~RageWeaponMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }
    RageWeaponSelector& selector() noexcept {
        return *selector_;
    }

private:
    Tab& tab_;
    Category category_;
    RageWeaponSelector* selector_ = nullptr;
};

// RAGE / Aimbot, 0x4348713C..0x43487B07. Descriptors are owned by the
// supplied runtime registry. Construction deliberately leaves dependent
// controls hidden until the original configuration-loaded lifecycle runs.
class RageAimbotMenu final : public BoundMenuControls {
public:
    RageAimbotMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities);
    ~RageAimbotMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    static void hitboxes_changed(void* context, Widget& source);
    static void multipoint_changed(void* context, Widget& source);
    Tab& tab_;
    Category category_;
    Checkbox* prefer_body_ = nullptr;
    Slider* multipoint_scale_ = nullptr;
    std::array<VisibilityLink, 7> dependencies_{};
};

// RAGE / Other, 0x43487B08..0x43487F7C, including the independently
// recovered QuickPeekMenu at its exact position in this category.
class RageOtherMenu final : public BoundMenuControls {
public:
    RageOtherMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities);
    ~RageOtherMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }
    const QuickPeekMenu& quick_peek() const noexcept {
        return *quick_peek_;
    }

private:
    Tab& tab_;
    Category category_;
    std::unique_ptr<QuickPeekMenu> quick_peek_;
};
} // namespace skeet::ui
