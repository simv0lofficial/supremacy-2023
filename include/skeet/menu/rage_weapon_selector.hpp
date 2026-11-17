#pragma once
#include "skeet/combobox.hpp"
#include "skeet/config.hpp"
#include "skeet/popup_menu.hpp"

namespace skeet::ui {
struct RageWeaponSelectorActions {
    void* context = nullptr;
    void (*switch_group)(void*, bool force_reload) = nullptr;
    void (*copy_group)(void*, std::uint32_t) = nullptr;
    // False means the native clipboard decoder returned an empty buffer.
    bool (*paste_group)(void*, std::uint32_t) = nullptr;
};
struct RageSelectorIcon {
    std::int32_t texture = -1;
    Point size{};
};
struct RageSelectorIconProvider {
    void* context = nullptr;
    bool (*load)(void*, RenderQueue&, std::uint8_t symbol, Point requested,
                 RageSelectorIcon&) = nullptr;
};

class RageWeaponSelector final : public ComboBox {
public:
    RageWeaponSelector();
    RageWeaponSelector(Widget* owner, std::u16string_view label, std::uint32_t* selection,
                       WidgetIdentities& identities, std::span<std::uint8_t> enabled_groups,
                       RageWeaponSelectorActions actions);
    RageWeaponSelector(Widget* owner, std::u16string_view label, std::uint32_t* selection,
                       WidgetIdentities& identities, std::span<BooleanConfig*, 12> enabled_groups,
                       RageWeaponSelectorActions actions);
    std::array<std::uint8_t*, 12> enabled_groups{};
    RageWeaponSelectorActions actions;
    RageSelectorIconProvider icons;
    Point icon_size{76, 20};
    std::int32_t global_icon_size = 32, checkbox_size = 8;
    std::uint32_t context_group = UINT32_MAX;
    PopupMenu context_menu;

    void bind_enabled_groups(std::span<std::uint8_t> values);
    void bind_enabled_groups(std::span<BooleanConfig*, 12> descriptors);

    bool hit_test(Point cursor) const override;
    bool mouse(const InputMessage& message, Point cursor) override;
    void interact_options(const InputMessage& message, Point cursor);
    void value_changed() override;
    void reset_input() override;
    void resize(const SliderMetrics& metrics) override;
    void dpi_changed(float factor, const SliderMetrics& metrics) override;
    void on_layout() override;
    void draw(RenderQueue& queue, Point origin, std::uint32_t accent) const override;
    void draw_option(RenderQueue& queue, Point position, std::int32_t height, std::uint32_t index,
                     std::uint32_t flags, std::uint32_t accent) const;
    void draw_checkbox(RenderQueue& queue, Point position, std::uint32_t group, std::int32_t height,
                       std::uint32_t accent) const;
    void context_selected(std::uint32_t index);

private:
    void initialize_selector();
    static void select_context(Widget&, PopupMenu&, std::uint32_t index);
    bool combo_mouse(const InputMessage& message, Point cursor);
    void draw_combo(RenderQueue& queue, Point origin, std::uint32_t accent) const;
};
}
