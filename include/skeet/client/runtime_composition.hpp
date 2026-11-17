#pragma once
#include "skeet/engine/source_initialization_imports.hpp"
#include "skeet/engine/hook_descriptors.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/input/window_hook.hpp"
#include <memory>

namespace skeet::engine {
class HookDescriptorStorage;
class StartupVectorCleanupBindings;
class StartupContainerCleanup;
}
namespace skeet::rage {
class GroupStorage;
class SpreadStorage;
struct SpreadStorageBindings;
struct GroupInitializerBindings;
}
namespace skeet::esp {
class FlagTable;
class FlagTableInitialization;
}
namespace skeet::script {
class UiTypeStorage;
struct UiTypeInitialization;
}
namespace skeet::events {
class EventNameStartupBindings;
}
namespace skeet {
struct MovementGlobalConfig;
}
namespace skeet::rage {
struct Settings;
}
namespace skeet::skins {
struct Settings;
}
namespace skeet::anti_aim {
struct Config;
struct GlobalConfig;
struct FakeLagSettings;
struct OtherGlobalSettings;
struct StartupInitializerBindings;
}
namespace skeet::esp {
struct PlayerEspSettings;
struct OtherEspSettings;
struct EffectsSettings;
struct ColoredModelsSettings;
}
namespace skeet::misc {
struct FovConfig;
struct KnifeConfig;
struct ZeusConfig;
struct AutomaticWeaponsConfig;
struct QuickSwitchConfig;
struct RevealRanksConfig;
struct RevealOverwatchConfig;
struct AutoAcceptConfig;
struct ClanTagConfig;
struct EventLogConfig;
struct GrenadePredictionConfig;
struct PersistentKillFeedConfig;
struct DisablePureConfig;
struct RebuyConfig;
struct ConsoleOutputConfig;
struct NoFallDamageConfig;
struct PingSpikeConfig;
struct FreeLookConfig;
struct AntiUntrustedConfig;
struct HideFromObsConfig;
struct CustomEventsConfig;
}
namespace skeet::ui {
struct WidgetIdentities;
class SettingsMenuStorage;
class RendererAssets;
}

namespace skeet::client {
class ClientRuntimeStorage;
class NativeStartupSources;
class StaticCleanupRuntime;
class HookDescriptorCleanupContext;
struct ExitRegistrationContext;
struct ConfigInitializerBindings;
struct GlobalConfigInitializerBindings;
struct TargetMaskSettings;
struct StaticCrtInitialization;

// These references name process-lifetime imported RNG/math services. The
// explicit overload also supports authored external-boundary fixtures.
struct CrtRuntimeImports {
    engine::SourceInitializationImports startup;
    engine::HookDescriptorSystem hook_queries;
    const rage::SpreadRandom& random;
    const RageTrigonometry& trigonometry;
};
const rage::SpreadRandom& native_spread_random();
CrtRuntimeImports native_crt_runtime_imports();

// Concrete ownership for the data/CRT portion of NativeStartupSourceGraph.
// Construction creates canonical views and their stable binding contexts;
// it runs no CRT callback, feature constructor, hook installation or menu.
// The full feature graph and exact final initializer remain required before
// NativeStartupRuntime can pass its whole-plan preflight.
// Storage and imported RNG/math referents must outlive this owner. Keep this
// owner alive until all callbacks registered in storage.exit_callbacks finish.
class CrtRuntimeOwner {
public:
    explicit CrtRuntimeOwner(ClientRuntimeStorage&);
    CrtRuntimeOwner(ClientRuntimeStorage&, CrtRuntimeImports);
    ~CrtRuntimeOwner();
    CrtRuntimeOwner(const CrtRuntimeOwner&) = delete;
    CrtRuntimeOwner& operator=(const CrtRuntimeOwner&) = delete;
    CrtRuntimeOwner(CrtRuntimeOwner&&) = delete;
    CrtRuntimeOwner& operator=(CrtRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    const rage::SpreadRandom& random() const noexcept;
    const RageTrigonometry& trigonometry() const noexcept;
    rage::SpreadStorage& spread() noexcept;
    const rage::SpreadStorageBindings& spread_bindings() const noexcept;
    StaticCleanupRuntime& static_cleanup() noexcept;
    ExitRegistrationContext& exit_registration() noexcept;
    engine::HookDescriptorStorage& descriptor_storage() noexcept;
    engine::HookDescriptorRuntime& descriptors() noexcept;
    engine::HookDescriptorSystem hook_system() const noexcept;
    HookDescriptorCleanupContext& descriptor_initializers() noexcept;
    rage::GroupStorage& rage_groups() noexcept;
    rage::GroupInitializerBindings& group_initializers() noexcept;
    esp::FlagTable& esp_flags() noexcept;
    esp::FlagTableInitialization& flag_initializers() noexcept;
    script::UiTypeStorage& ui_types() noexcept;
    script::UiTypeInitialization& ui_type_initializers() noexcept;
    events::EventNameStartupBindings& event_name_initializers() noexcept;
    engine::FirstStartupServiceBindings& first_startup_services() noexcept;

    // Bind only the72 complete CRT bodies owned here. This supplies no final
    // initializer and never executes an initializer or no-argument finalizer.
    // Full graph construction normally invokes the same individual binders.
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};

// A non-owning view of the one set of source feature settings. Constructors
// of their actual feature owners retain registration/default-write order.
struct MiscFeatureSettings {
    misc::FovConfig& fov;
    misc::KnifeConfig& knife;
    misc::ZeusConfig& zeus;
    misc::AutomaticWeaponsConfig& automatic_weapons;
    misc::QuickSwitchConfig& quick_switch;
    misc::RevealRanksConfig& reveal_ranks;
    misc::RevealOverwatchConfig& reveal_overwatch;
    misc::ClanTagConfig& clan_tag;
    misc::EventLogConfig& event_log;
    misc::GrenadePredictionConfig& grenades;
    misc::PersistentKillFeedConfig& persistent_kill_feed;
    misc::DisablePureConfig& disable_pure;
    misc::RebuyConfig& rebuy;
    misc::ConsoleOutputConfig& console_output;
    misc::NoFallDamageConfig& no_fall_damage;
};

// Stable canonical source descriptors for CRT, feature constructors and menu
// lookup. Composite source bundles are owned once here; consumers borrow
// these fields rather than reading independent native-address placeholders.
// The five single-descriptor global views and raw GUI settings borrow their
// exact data cells, including the HUD's live43468E58 hide flag. Construction
// preserves those cells and performs no descriptor/hotkey registration.
class ConfigurationRuntimeOwner {
public:
    explicit ConfigurationRuntimeOwner(ClientRuntimeStorage&);
    ~ConfigurationRuntimeOwner();
    ConfigurationRuntimeOwner(const ConfigurationRuntimeOwner&) = delete;
    ConfigurationRuntimeOwner& operator=(const ConfigurationRuntimeOwner&) = delete;
    ConfigurationRuntimeOwner(ConfigurationRuntimeOwner&&) = delete;
    ConfigurationRuntimeOwner& operator=(ConfigurationRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    MovementGlobalConfig& movement() noexcept;
    skins::Settings& skins_settings() noexcept;
    rage::Settings& rage_settings() noexcept;
    anti_aim::Config& anti_aim_settings() noexcept;
    anti_aim::GlobalConfig& anti_aim_global() noexcept;
    anti_aim::FakeLagSettings& fake_lag_settings() noexcept;
    anti_aim::OtherGlobalSettings& anti_aim_other() noexcept;
    esp::PlayerEspSettings& player_esp_settings() noexcept;
    esp::OtherEspSettings& other_esp_settings() noexcept;
    esp::EffectsSettings& effects_settings() noexcept;
    esp::ColoredModelsSettings& colored_model_settings() noexcept;
    MiscFeatureSettings& miscellaneous() noexcept;
    misc::PingSpikeConfig& ping_spike() noexcept;
    misc::FreeLookConfig& free_look() noexcept;
    misc::AntiUntrustedConfig& anti_untrusted() noexcept;
    misc::HideFromObsConfig& hide_from_obs() noexcept;
    misc::CustomEventsConfig& custom_events() noexcept;
    TargetMaskSettings& target_mask() noexcept;
    ui::WidgetIdentities& widget_identities() noexcept;
    ui::SettingsMenuStorage& settings_menu() noexcept;
    anti_aim::StartupInitializerBindings& anti_aim_initializers() noexcept;
    ConfigInitializerBindings& static_initializers() noexcept;
    GlobalConfigInitializerBindings& global_initializers() noexcept;
    // Bind only the58 existing independent descriptor bodies. Movement,
    // shared skins and the menu hotkey are composed by the full source graph.
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};

struct WindowRuntimeImports {
    engine::SourceInitializationImports startup;
    engine::UnloadSystemFunctions unload;
    input::WindowHookImports subclass;
};
WindowRuntimeImports native_window_runtime_imports(const engine::SourceInitializationImports&);

// One process hook/unload state, available before ScriptOwner and MainMenu.
// Engine/device-manager pointers, lifecycle flags, both patch-list heads and
// the optional connection callback borrow the actual source data cells;
// later publication is observed without refresh.
// Construction installs no subclass, calls no window procedure and registers
// no callback. The explicit bind operation only publishes source callback
// context; PostInitializationRuntime retains the actual install position.
class WindowRuntimeOwner {
public:
    explicit WindowRuntimeOwner(ClientRuntimeStorage&);
    WindowRuntimeOwner(ClientRuntimeStorage&, const engine::SourceInitializationImports&);
    WindowRuntimeOwner(ClientRuntimeStorage&, WindowRuntimeImports);
    ~WindowRuntimeOwner();
    WindowRuntimeOwner(const WindowRuntimeOwner&) = delete;
    WindowRuntimeOwner& operator=(const WindowRuntimeOwner&) = delete;
    WindowRuntimeOwner(WindowRuntimeOwner&&) = delete;
    WindowRuntimeOwner& operator=(WindowRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    engine::UnloadState& unload_state() noexcept;
    engine::UnloadRuntime& unload_runtime() noexcept;
    input::WindowSubclassRuntime& subclass() noexcept;
    const engine::SourceInitializationImports& startup_imports() const noexcept;
    void bind_subclass_callback();

private:
    struct Data;
    std::unique_ptr<Data> data_;
};

// Explicit source inputs for the two otherwise unconsumed incoming-ECX words
// retained by the native inline-container constructors. No value is inferred
// from a captured context or supplied by a default argument.
struct StaticCrtConstructorInputs {
    std::uint32_t flags_word;
    std::uint32_t keys_word;
};
struct StaticCrtImports {
    void (*dispose_v8_global)(void**);
};
StaticCrtImports native_static_crt_imports();

// The remaining CRT contexts borrow early icon/listener storage and the same
// process unload state. No device, renderer, codecs or engine interfaces are
// required here. Native container headers remain native header views;
// they do not own or release the separate source Lua/catalog/preset vectors.
// A source bootstrap must select SourceTerminalCleanupBindings for its actual
// source container producers before executing any CRT registration callback.
class StaticCrtRuntimeOwner {
public:
    StaticCrtRuntimeOwner(CrtRuntimeOwner&, WindowRuntimeOwner&, ui::RendererAssets&,
                          StaticCrtConstructorInputs);
    StaticCrtRuntimeOwner(CrtRuntimeOwner&, WindowRuntimeOwner&, ui::RendererAssets&,
                          StaticCrtConstructorInputs, StaticCrtImports);
    ~StaticCrtRuntimeOwner();
    StaticCrtRuntimeOwner(const StaticCrtRuntimeOwner&) = delete;
    StaticCrtRuntimeOwner& operator=(const StaticCrtRuntimeOwner&) = delete;
    StaticCrtRuntimeOwner(StaticCrtRuntimeOwner&&) = delete;
    StaticCrtRuntimeOwner& operator=(StaticCrtRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    StaticCrtInitialization& static_initializers() noexcept;
    engine::StartupVectorCleanupBindings& native_vector_cleanup() noexcept;
    engine::StartupContainerCleanup& native_container_cleanup() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
