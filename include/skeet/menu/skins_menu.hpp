#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/listbox.hpp"
#include "skeet/skins/config.hpp"
#include "skeet/skins/catalog.hpp"

namespace skeet::ui {
// SKINS, native builder 0x4348BF79..0x4348CB44.
class SkinsMenu final : public BoundMenuControls {
public:
    SkinsMenu(Tab&, ConfigRegistries&, WidgetIdentities&, skins::Catalog&, skins::Refresh&);
    ~SkinsMenu();
    std::array<Category*, 2> categories() noexcept {
        return {&models_, weapon_.get()};
    }
    void select_weapon(std::int32_t item, std::int32_t team = -1);
    void select_team(std::int32_t team);
    void rebuild_skin_list();
    void initialize_schema_options();
    void rebuild_glove_paint_list();
    void activate(); // 0x4331FB95
    void initialize_default_paints(); // 0x433D7108
    void set_agent_team(std::int32_t team);
    std::int32_t selected_weapon() const noexcept {
        return selected_weapon_;
    }
    std::int32_t selected_team() const noexcept {
        return selected_team_;
    }

private:
    static void team_changed(void*, Widget&);
    static void weapon_changed(void*, Widget&);
    static void filter_changed(void*, Widget&);
    static void enabled_changed(void*, Widget&);
    static void paint_changed(void*, Widget&);
    static void knife_enabled_changed(void*, Widget&);
    static void knife_model_changed(void*, Widget&);
    static void agent_enabled_changed(void*, Widget&);
    static void glove_model_changed(void*, Widget&);
    static void glove_enabled_changed(void*, Widget&);
    void refresh();
    void rebuild_glove_paint_list(std::uint16_t item, std::uint32_t previous);
    Tab& tab_;
    ConfigRegistries& settings_;
    skins::Catalog& catalog_;
    skins::Refresh& refresh_;
    Category models_;
    std::unique_ptr<Category> weapon_;
    std::int32_t selected_weapon_ = 0, selected_team_ = 3;
    std::array<std::uint32_t, 32> enabled_bits_{};
    std::uint8_t knife_state_ = 0, glove_state_ = 0, mask_state_ = 0;
    Slider *team_ = nullptr, *weapon_selector_ = nullptr, *quality_ = nullptr, *seed_ = nullptr;
    Checkbox *knife_ = nullptr, *glove_ = nullptr, *mask_ = nullptr, *agent_ = nullptr;
    Checkbox *enabled_ = nullptr, *stattrak_ = nullptr, *filter_ = nullptr;
    ComboBox *knife_model_ = nullptr, *glove_model_ = nullptr, *glove_paint_ = nullptr,
             *mask_model_ = nullptr;
    ComboBox *agent_t_ = nullptr, *agent_ct_ = nullptr;
    Listbox* skin_ = nullptr;
    std::array<VisibilityLink, 4> dependencies_{};
};
} // namespace skeet::ui
