#pragma once
#include "skeet/client_menu.hpp"

namespace skeet::ui {
// AA / Fake lag, 0x434885A6..0x434886CD, and Other,
// 0x434886D1..0x43488833. Bind the caller-owned native settings.
class FakeLagMenu final : public BoundMenuControls {
public:
    FakeLagMenu(Tab&, ConfigRegistries&, WidgetIdentities&);
    ~FakeLagMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }
    Slider& limit_slider() noexcept {
        return *limit_;
    }

private:
    Tab& tab_;
    Category category_;
    Slider* limit_ = nullptr;
};
class AntiAimOtherMenu final : public BoundMenuControls {
public:
    AntiAimOtherMenu(Tab&, ConfigRegistries&, WidgetIdentities&);
    ~AntiAimOtherMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    Tab& tab_;
    Category category_;
};
} // namespace skeet::ui
