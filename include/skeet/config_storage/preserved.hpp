#pragma once
#include "owner.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/engine/jobs.hpp"

namespace skeet::ui {
struct MainMenuRegistryName;
class RootMenu;
}
namespace skeet {
struct NativeConfigVector {
    std::uint32_t begin, end, capacity;
};
static_assert(sizeof(NativeConfigVector) == 12);
struct SettingsRegistryWindow {
    const ui::MainMenuRegistryName* name = nullptr;
};
struct NativeSettingsRegistryWindow {
    std::uint32_t address;
};

// A view of native CONFIG BSS cells in the retained image. settings() binds
// values only after their actual owner has been constructed. Binding never
// calls a constructor, decodes a context record, relocates data or supplies
// hashes. The retained image/environment owns the actual BSS and heap lifetime.
// initialize_settings() creates the recovered source owner in43478C68;
// binding an already constructed owner does not repeat that constructor.
class PreservedConfigStorage {
public:
    explicit PreservedConfigStorage(engine::PreservedInitialization&);
    explicit PreservedConfigStorage(std::shared_ptr<engine::NativeDataImage>);
    PreservedConfigStorage(const PreservedConfigStorage&) = delete;
    PreservedConfigStorage& operator=(const PreservedConfigStorage&) = delete;
    SettingsPersistenceState& initialize_settings(engine::JobMemory);
    SettingsPersistenceState& settings();
    ConfigRegistries& configuration() noexcept {
        return configuration_;
    }
    NativeConfigPath& path() const;
    NativeConfigVector& baseline_cells() const;
    std::span<const std::byte> baseline() const;
    // Source publication43476924 contains a RootMenu controller. Its registry
    // name belongs to the concrete main-window lifetime, not its C++ layout.
    void bind_source_menu_registry(ui::RootMenu&, const ui::MainMenuRegistryName&);
    SettingsRegistryWindow capture_registry_window() const; //4340BA52, before433EEC8B
    std::u16string_view registry_value_name(SettingsRegistryWindow) const;
    // Explicit captured-native inspection route. Source persistence never
    // selects this route from a null binding or by catching an exception.
    NativeSettingsRegistryWindow capture_native_registry_window() const;
    std::u16string_view native_registry_value_name(NativeSettingsRegistryWindow) const;
    // Actual ordinary terminal callbacks; neither is an automatic destructor.
    void finalize_settings(const engine::JobMemory&); //4332EFAD: free, then slot=0
    void finalize_baseline(const engine::JobMemory&); //43329A5B: free, no cell reset
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    ConfigRegistries configuration_;
    std::unique_ptr<SettingsPersistenceState> settings_;
    bool baseline_finalized_ = false;
    ui::RootMenu* source_menu_ = nullptr;
    const ui::MainMenuRegistryName* source_registry_name_ = nullptr;
    std::uint32_t& settings_cell() const;
};
} // namespace skeet
