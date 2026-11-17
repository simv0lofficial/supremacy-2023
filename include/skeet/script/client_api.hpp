#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/script/shutdown.hpp"
#include "skeet/core.hpp"
#include "skeet/misc/clan_tag.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/engine/loaded_modules.hpp"
#include "skeet/engine/hook_descriptors.hpp"
#include "skeet/engine/hook_descriptor_storage.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <variant>

namespace skeet::misc {
class NormalLog;
class ErrorLog;
class ColoredLogRing;
}
namespace skeet::ui {
class PlayersMenu;
}
namespace skeet::client {
struct ClientHitboxRuntime;
}
namespace skeet::script {
class ClientHudLogSource {
public:
    ClientHudLogSource(misc::ColoredLogRing* standalone = nullptr) noexcept : source_(standalone) {}
    static ClientHudLogSource canonical(misc::ColoredLogRing* const& slot) noexcept {
        return ClientHudLogSource(&slot);
    }
    misc::ColoredLogRing* get() const noexcept {
        if (const auto* slot = std::get_if<misc::ColoredLogRing* const*>(&source_))
            return **slot;
        return std::get<misc::ColoredLogRing*>(source_);
    }
    explicit operator bool() const noexcept {
        return get() != nullptr;
    }
    misc::ColoredLogRing* operator->() const noexcept {
        return get();
    }

private:
    explicit ClientHudLogSource(misc::ColoredLogRing* const* slot) noexcept : source_(slot) {}
    std::variant<misc::ColoredLogRing*, misc::ColoredLogRing* const*> source_;
};
struct ClientOffsets {
    std::ptrdiff_t life_state = 0; //9D920E35
    std::ptrdiff_t movement_type = 0; //7501CB91
    std::ptrdiff_t observer_mode = 0; //2F4E97C4
    std::ptrdiff_t observer_target = 0; //3B704B41
    std::ptrdiff_t view_offset = 0; //C246315E
    std::ptrdiff_t active_weapon = 0; //CD812654
    std::ptrdiff_t item_definition = 0;
    std::ptrdiff_t collision_group = 0; //E4E2D1A7
    std::ptrdiff_t owner_handle = 0; //49D93E62-18
    std::ptrdiff_t valve_matchmaking = 0; //BF567619
};
struct ClientRandom {
    // Genuine vstdlib imports; the module implements the client's seed gate.
    void (*seed)(std::int32_t) = nullptr;
    std::int32_t (*integer)(std::int32_t, std::int32_t) = nullptr;
    float (*uniform)(float, float) = nullptr;
};
struct ClientClock {
    std::int64_t origin = 0; //4347BD40
    double milliseconds_per_count = 0; //43479CE8
    bool (*counter)(std::int64_t&) = nullptr;
    std::uint64_t (*system_time)() = nullptr;
    std::uint64_t (*time_zone_bias)() = nullptr;
    std::int32_t (*unix_seconds)() = nullptr;
};
void initialize_client_clock(ClientClock&); //4347E307
ClientClock
client_clock_imports() noexcept; // imports only; actual process initializer samples time
ClientClock system_client_clock();

// Shared434758BC..C8 state. The replaced pointer is the engine console
// object's complete vtable, rather than a callback at one vtable entry.
struct ConsoleInputHook {
    ConsoleInputHook(engine::HookDescriptorStorage& storage, Bridge& runtime);
    engine::HookDescriptorStorage& storage;
    engine::VtableHookMetadata& metadata; //same434758B8 descriptor
    Bridge& bridge;
    void capture(void* engine_container, void* callback_owner); //43482378
    void enable() const; //console_input branch433E2344
    void restore() const; //433F8C58
    void process(void* engine_object, const char* text) const; //433410B6,owner slot9
    // Actual recipe24: clone client RTTI46E07364/base0, replace slot279.
    void prepare(engine::NativeImageEnvironment, engine::JobMemory, engine::HookDescriptorSystem);
    void clear(engine::JobMemory); //actualdescriptor cleanup433D2DC1
};
bool dispatch_boolean_callbacks(Bridge&, const char* name, Marshaller, void* context); //433309E4
bool dispatch_console_input(Bridge&, const char* text); //433B2646
#if defined(_M_IX86) || defined(__i386__)
void __fastcall console_input_hook(void* object, void*, const char* text); //4334E722
#else
void console_input_hook(void* object, void*, const char* text);
#endif

struct ClientBindings {
    void* engine = nullptr;
    void* entity_list = nullptr;
    void* cvar = nullptr;
    void* debug_overlay = nullptr;
    void* model_info = nullptr;
    void* const& local_player; //live global43475808
    engine::ConVarBinding latency_rate_cvar = nullptr; //live4346747C ^03B4819C
    ClientOffsets offsets{};
    InputBitmap input{};
    std::array<float, 3>* camera_position = nullptr; //43479CCC
    const double* real_latency_incoming = nullptr; //4347B118
    const double* real_latency_outgoing = nullptr; //4347A500
    const std::int32_t* frame_number = nullptr; //43475B2C
    const float* interval_per_tick = nullptr;
    const float* engine_interval = nullptr; //43467B04+20
    std::int32_t* threat_handle = nullptr; //4346DB1C
    std::uint8_t* reload_scripts = nullptr; //4347CA28
    std::uint8_t* full_update = nullptr; //43475BD2
    std::uint8_t* subscriptions_dirty = nullptr; //43468364+A0
    const std::uint8_t* angle_restriction_switch = nullptr; //434693EC
    void** const& game_rules; //live outer global4346A3A4
    ConsoleInputHook* console_input = nullptr;
    const std::uint8_t* unsafe_access = nullptr;
    const std::uint8_t* unsafe_builtin = nullptr; //43475A5C
    const bool* initializing = nullptr; //4347582C
    const engine::LoadedModuleEnvironment* loaded_modules = nullptr;
    ClientRandom random{};
    ClientClock clock{};
    misc::SetClanTag set_clan_tag = nullptr;
    misc::NormalLog* normal_log = nullptr;
    misc::ErrorLog* error_log = nullptr;
    ClientHudLogSource hud_log{}; //live434758CC, constructed after ScriptOwner
    ui::PlayersMenu* players_menu = nullptr;
    BallisticEngine* ballistics = nullptr;
    void** bullet_filter_vtable = nullptr; //actual engine vtable43475B90
    client::ClientHitboxRuntime* hitboxes = nullptr;
};

class ClientModule {
public:
    explicit ClientModule(ClientBindings& bindings, ReferenceStrings& flags) noexcept
        : bindings(bindings), flags(flags) {}
    ClientBindings& bindings;
    ReferenceStrings& flags;
    void register_exports(Bridge&); //433422D2, all37 native exports
    enum class Export : std::uint32_t {
        set_event_callback,
        unset_event_callback,
        fire_event,
        log,
        color_log,
        error_log,
        exec,
        userid_to_entindex,
        get_cvar,
        set_cvar,
        random_int,
        random_float,
        draw_debug_text,
        draw_hitboxes,
        screen_size,
        visible,
        trace_line,
        trace_bullet,
        scale_damage,
        latency,
        real_latency,
        camera_angles,
        camera_position,
        eye_position,
        timestamp,
        set_clan_tag,
        system_time,
        unix_time,
        reload_active_scripts,
        create_interface,
        find_signature,
        key_state,
        update_player_list,
        get_model_name,
        register_esp_flag,
        current_threat,
        request_full_update
    };
    int invoke(Export, void*);
    int callbacks(Export, void*);
    int logging(Export, void*);
    int queries(Export, void*);
    int tracing(Export, void*);
    int diagnostics(Export, void*);
    int unsafe(Export, void*);
    Bridge& bridge() const noexcept {
        return *bridge_;
    }
    void* eye_player() const; //43359AFA
    std::span<const std::uint32_t> event_subscriptions() const noexcept {
        return subscribed_events_;
    }
    //433CDB46/433D71C7: receives the actual engine GameEvent object.
    void dispatch_engine_event(void* event);

private:
    Bridge* bridge_ = nullptr;
    std::int32_t last_clan_frame_ = 0; //4347CA40
    std::int32_t last_player_update_ = 0; //4347CA44
    std::uint32_t random_seed_frame_ = 0; //4346D564
    std::vector<std::uint32_t> subscribed_events_;
    std::unordered_map<std::string, void*> cvar_cache_;
    static int entry(void*);
    bool builtin_event(std::uint32_t) const noexcept;
    void subscribe(const char*);
    void seed_random(); //4341A641
    void* client_cvar(const char*); //433B90BE
};
} // namespace skeet::script
