#pragma once
#include "skeet/client_menu.hpp"
#include "skeet/players/state.hpp"
#include "skeet/button.hpp"
#include "skeet/listbox.hpp"

namespace skeet::ui {

class PlayersMenu final : public BoundMenuControls {
public:
    PlayersMenu(Tab&, WidgetIdentities&, players::State&, players::Bindings&);
    ~PlayersMenu();
    Category& players_category() noexcept {
        return players_;
    }
    Category& adjustments_category() noexcept {
        return *adjustments_;
    }
    Listbox& player_list() noexcept {
        return *list_;
    }
    std::int32_t selected_player = 0; // native tab+98
    void activate(); // 433ECDDF / 4338936B
    void reset_all();
    void apply_to_all();

private:
    static void selection_changed(void*, Widget&);
    static void reset_clicked(void*, Widget&);
    static void apply_clicked(void*, Widget&);
    void enable_category(Category&, bool);
    Tab& tab_;
    Category players_;
    std::unique_ptr<Category> adjustments_;
    players::State& state_;
    players::Bindings& runtime_bindings_;
    Listbox* list_ = nullptr;
    Checkbox *whitelist_ = nullptr, *shared_ = nullptr, *visuals_ = nullptr, *priority_ = nullptr,
             *force_pitch_ = nullptr, *force_body_ = nullptr, *correction_ = nullptr;
    Slider *pitch_ = nullptr, *body_ = nullptr;
    ComboBox *prefer_body_ = nullptr, *safe_ = nullptr;
    std::array<VisibilityLink, 2> links_{};
};

} // namespace skeet::ui
