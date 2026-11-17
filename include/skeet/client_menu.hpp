#pragma once
#include "color_picker.hpp"
#include "combobox.hpp"
#include "hotkey_widget.hpp"
#include "menu_tree.hpp"
#include "multiselect.hpp"
#include "effects/camera.hpp"
#include <memory>
#include <utility>

namespace skeet::ui {

struct BoundControl {
    // Zero identifies an unbound static label in a recovered control group.
    std::uint32_t config_hash;
    Widget* widget;
};

// A group binds existing registry descriptors. It does not manufacture missing
// settings: every required descriptor must have its native type and lifetime.
class BoundMenuControls {
public:
    BoundMenuControls(const BoundMenuControls&) = delete;
    BoundMenuControls& operator=(const BoundMenuControls&) = delete;
    std::span<const BoundControl> controls() const noexcept {
        return bindings_;
    }
    Widget* find(std::uint32_t hash) const noexcept;

protected:
    BoundMenuControls() = default;
    template <class Control, class... Arguments>
    Control& add(Category& owner, std::uint32_t hash, Arguments&&... arguments) {
        auto control = std::make_unique<Control>(&owner, std::forward<Arguments>(arguments)...);
        auto& result = *control;
        storage_.push_back(std::move(control));
        bindings_.push_back({hash, &result});
        owner.elements.push_back(&result);
        return result;
    }
    void detach(Category& owner) noexcept;
    std::vector<std::unique_ptr<Widget>> storage_;
    std::vector<BoundControl> bindings_;
};

// MISC / Movement, 0x4348B5BB..0x4348BA32. The supplied tab and registered
// settings must outlive this object. Construction preserves the native hidden
// state; the existing Tab::config_loaded lifecycle refreshes dependencies.
class MovementMenu final : public BoundMenuControls {
public:
    MovementMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities);
    ~MovementMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    static void air_strafe_changed(void* context, Widget& source);
    static void z_hop_changed(void* context, Widget& source);
    void update_z_hop(const Checkbox& source);
    Tab& tab_;
    Category category_;
    MultiSelect* direction_ = nullptr;
    Slider* smoothing_ = nullptr;
    Checkbox* avoid_collisions_ = nullptr;
    Checkbox* z_hop_ = nullptr;
    HotkeyWidget* z_hop_key_ = nullptr;
    Checkbox* pre_speed_ = nullptr;
    HotkeyWidget* pre_speed_key_ = nullptr;
    Slider* z_hop_first_ = nullptr;
    Slider* z_hop_second_ = nullptr;
};

// RAGE's Quick peek block, 0x43487C95..0x43487DE5, appended at the supplied
// category's current position. Its three dependent controls use the native
// checkbox visibility list, including refresh after configuration loading.
class QuickPeekMenu final : public BoundMenuControls {
public:
    QuickPeekMenu(Category& category, ConfigRegistries& settings, WidgetIdentities& identities);
    ~QuickPeekMenu();

private:
    Category& category_;
    std::array<VisibilityLink, 3> dependencies_{};
};

// VISUALS / Player ESP, 0x43489391..0x434897F5. All 34 configuration
// descriptors belong to existing runtime settings; the static activation label
// is included in the original control order with config_hash == 0.
class PlayerEspMenu final : public BoundMenuControls {
public:
    PlayerEspMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities);
    ~PlayerEspMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    static void arrow_visibility_changed(void* context, Widget& source);
    Tab& tab_;
    Category category_;
    Slider* arrow_size_ = nullptr;
    Slider* arrow_distance_ = nullptr;
};

// VISUALS / Other ESP, 0x4348A2D1..0x4348ABAD. The Danger Zone tail is
// refreshed during construction; the other two visibility lists are refreshed
// by Checkbox::config_loaded, matching their distinct native lifecycles.
class OtherEspMenu final : public BoundMenuControls {
public:
    OtherEspMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities);
    ~OtherEspMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    static void danger_zone_changed(void* context, Widget& source);
    Tab& tab_;
    Category category_;
    std::array<VisibilityLink, 4> dependencies_{};
    Widget* last_danger_zone_control_ = nullptr;
};

// VISUALS / Colored models, 0x434897F6..0x4348A2C7. All eight material
// selectors use the same recovered seven choices. Secondary colors appear
// only for material IDs 2..5 while the corresponding model group is enabled.
class ColoredModelsMenu final : public BoundMenuControls {
public:
    ColoredModelsMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities);
    ~ColoredModelsMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    struct MaterialGroup {
        Checkbox* primary = nullptr;
        Checkbox* behind_wall = nullptr;
        ComboBox* material = nullptr;
        ColorPicker* secondary_color = nullptr;
        Slider* duration = nullptr;
    };
    static bool active(const MaterialGroup& group) noexcept;
    static void enabled_changed(void* context, Widget& source);
    static void material_changed(void* context, Widget& source);
    Tab& tab_;
    Category category_;
    std::array<MaterialGroup, 8> groups_{};
};

// VISUALS / Effects, 0x4348ABAE..0x4348B01C. The two camera callbacks
// execute the recovered engine-facing runtime against the same descriptors.
class EffectsMenu final : public BoundMenuControls {
public:
    EffectsMenu(Tab& tab, ConfigRegistries& settings, WidgetIdentities& identities,
                effects::CameraRuntime& camera);
    ~EffectsMenu();
    Category& category() noexcept {
        return category_;
    }
    const Category& category() const noexcept {
        return category_;
    }

private:
    static void brightness_changed(void* context, Widget& source);
    static void alive_camera_changed(void* context, Widget& source);
    static void dead_camera_changed(void* context, Widget& source);
    Tab& tab_;
    Category category_;
    effects::CameraRuntime& camera_;
    ColorPicker* night_color_ = nullptr;
    std::array<VisibilityLink, 2> dependencies_{};
};

} // namespace skeet::ui
