#pragma once
#include "skeet/misc/native_text.hpp"
#include <array>
#include <memory>

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::engine {
class PreservedInitialization;
class NativeDataImage;
}
namespace skeet::events {
struct EventRegistry;
class StandaloneEventNameStorage;

// Addresses are the actual first words of42 native16-byte string cells.
enum class EventName : std::uint32_t {
    pre_render = 0x43479360,
    pre_render_3d = 0x43479370,
    override_view = 0x43479380,
    string_cmd = 0x43479390,
    text = 0x434793A0,
    pre_config_load = 0x434793B0,
    post_config_load = 0x434793C0,
    pre_config_save = 0x434793D0,
    post_config_save = 0x434793E0,
    send_datagram = 0x434793F0,
    send_net_msg = 0x43479400,
    player_chat = 0x43479410,
    teamonly = 0x43479420,
    entity = 0x43479430,
    name = 0x43479440,
    server_cvar = 0x43479450,
    cvarname = 0x43479460,
    cvarvalue = 0x43479470,
    invalid_event_name = 0x43479480,
    hitgroup = 0x43479490,
    damage = 0x434794A0,
    hit_chance = 0x434794B0,
    boosted = 0x434794C0,
    high_priority = 0x434794D0,
    aim_fire = 0x434794E0,
    aim_hit = 0x434794F0,
    aim_miss = 0x43479500,
    console_input = 0x43479510,
    net_update_start = 0x43479520,
    net_update_end = 0x43479530,
    level_init = 0x43479540,
    run_command = 0x43479550,
    finish_command = 0x43479560,
    chokedcommands = 0x43479570,
    command_number = 0x43479580,
    post_render = 0x43479590,
    paint = 0x434795A0,
    paint_ui = 0x434795B0,
    pre_predict_command = 0x434795C0,
    predict_command = 0x434795D0,
    setup_command = 0x434795E0,
    voice = 0x434795F0
};

// Copies retain a binding to the native row, never its current pointer/string.
// Predefined consumers read get() at their original argument-read points.
class EventNameRef {
public:
    explicit EventNameRef(const misc::NativeText& cell) noexcept : cell_(cell) {}
    const char* get() const noexcept {
        return cell_.begin;
    }
    operator const char*() const noexcept {
        return get();
    }
    const misc::NativeText& native() const noexcept {
        return cell_;
    }

private:
    const misc::NativeText& cell_;
};

class EventNameStorage {
    friend class StandaloneEventNameStorage;
    std::shared_ptr<engine::NativeDataImage> image_;

public:
    static constexpr std::uint32_t native_base = 0x43479360;
    static constexpr std::size_t count = 42;
    using Cells = std::array<misc::NativeText, count>;
    explicit EventNameStorage(engine::PreservedInitialization&);
    explicit EventNameStorage(std::shared_ptr<engine::NativeDataImage>);
    EventNameStorage(const EventNameStorage&) = delete;
    EventNameStorage& operator=(const EventNameStorage&) = delete;
    EventNameStorage(EventNameStorage&&) = delete;
    EventNameStorage& operator=(EventNameStorage&&) = delete;
    Cells& cells; //required actual image-native43479360 cells; never reset here
    const engine::NativeDataImage& image() const;
    std::shared_ptr<engine::NativeDataImage> image_owner() const noexcept {
        return image_;
    }
    misc::NativeText& at(EventName);
    const misc::NativeText& at(EventName) const;
    EventNameRef alias(EventName name) const {
        return EventNameRef(at(name));
    }

private:
    explicit EventNameStorage(Cells& snapshot) noexcept : cells(snapshot) {}
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(EventNameStorage::Cells) == 0x2A0);
#endif

// Explicit isolated fixture ownership. Production cannot obtain this private
// constructor through a default or detached singleton. image() rejects this
// snapshot, so it cannot pass the live startup graph's image identity guard.
class StandaloneEventNameStorage {
    // Longest constructor operand is19 visible bytes. These are explicit
    // consumer-fixture buffers, not allocations or default production names.
    std::array<std::array<char, 20>, EventNameStorage::count> backing_{};
    EventNameStorage::Cells cells_{};
    EventNameStorage view_;

public:
    StandaloneEventNameStorage() noexcept;
    StandaloneEventNameStorage(const StandaloneEventNameStorage&) = delete;
    StandaloneEventNameStorage& operator=(const StandaloneEventNameStorage&) = delete;
    StandaloneEventNameStorage(StandaloneEventNameStorage&&) = delete;
    StandaloneEventNameStorage& operator=(StandaloneEventNameStorage&&) = delete;
    EventNameStorage& storage() noexcept {
        return view_;
    }
    const EventNameStorage& storage() const noexcept {
        return view_;
    }
};
// All production consumers borrow the one retained image view. Allocation
// occurs only at43480770, not when that view or its registry is constructed.
EventNameStorage& initialize_event_name_storage(EventNameStorage&, engine::JobMemory); //43480770
void finalize_event_name_storage(EventNameStorage&, engine::JobMemory); //43419B17 ->4332B67C

class EventNameStartupBindings {
public:
    EventNameStartupBindings(EventRegistry& registry, client::ExitCallbacks& callbacks,
                             engine::JobMemory memory) noexcept
        : registry(registry), callbacks(callbacks), memory(memory) {}
    EventNameStartupBindings(const EventNameStartupBindings&) = delete;
    EventNameStartupBindings& operator=(const EventNameStartupBindings&) = delete;
    EventNameStartupBindings(EventNameStartupBindings&&) = delete;
    EventNameStartupBindings& operator=(EventNameStartupBindings&&) = delete;
    EventRegistry& registry;
    client::ExitCallbacks& callbacks;
    engine::JobMemory memory;
    void initialize();
    void finalize();
    void (*source_finalizer(std::uint32_t native_address))();
};
// Individual4331EDEC/operand1F4: construct all rows, then append43419B17 to
// the SAME existing433837F5 exit registry. Binding does not execute startup.
void bind_event_name_initializer(client::NativeStartupSources&, EventNameStartupBindings&);
} // namespace skeet::events
