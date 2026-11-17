#pragma once
#include "config.hpp"
#include "logs.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/render_queue.hpp"
#include <cstddef>

namespace skeet::misc {
// The five fields are the actual 433877FF stack record. The two byte pointers
// alias the caller's SendNetMsg arguments; cancellation is tested after each
// registered callback, before advancing to the next callback.
struct NetworkMessageCall {
    void* channel;
    void* message;
    std::uint8_t* force_reliable;
    std::uint8_t* voice;
    std::uint8_t cancelled;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(NetworkMessageCall, message) == 4);
static_assert(offsetof(NetworkMessageCall, force_reliable) == 8);
static_assert(offsetof(NetworkMessageCall, voice) == 0xC);
static_assert(offsetof(NetworkMessageCall, cancelled) == 0x10);
static_assert(sizeof(NetworkMessageCall) == 0x14);
#endif
#if defined(_M_IX86) || defined(__i386__)
using NetworkMessageEntry = void(__fastcall*)(void*, NetworkMessageCall*);
#else
using NetworkMessageEntry = void (*)(void*, NetworkMessageCall*);
#endif
struct NetworkMessageCallback {
    const NetworkMessageEntry* vtable;
};

// Secondary endpoint of owner 434864C4. The configuration is the canonical
// descriptor 177AB88C, and registration uses 43475B94/B98/BC (4336F73C).
struct DisableSvPureHandler : NetworkMessageCallback {
    explicit DisableSvPureHandler(const DisablePureConfig&) noexcept;
    DisableSvPureHandler(engine::OwnerRegistryBinding, const DisablePureConfig&);
    void initialize_vtable() noexcept;
    DisableSvPureHandler(const DisableSvPureHandler&) = delete;
    DisableSvPureHandler& operator=(const DisableSvPureHandler&) = delete;
    const DisablePureConfig& config;
};
void apply_disable_sv_pure(const DisablePureConfig&, NetworkMessageCall&); // complete4335C233

// All members refer to the actual shared globals, including the reset entry
// also used by UnloadState::in_game_callback. No handshake state is copied.
struct NetworkMessageTransition {
    std::uint8_t& active; // 43475BD2
    const std::int32_t*& sequence; // 434758D8
    std::uint8_t& mismatched; // 4347C9F0
    std::int32_t& status; // 4347C9EC
    float& acknowledged_at; // 43479924
    const float*& current_time; // 4346A334
    void (*&reset)(); // 43468BF0
};
void update_network_message_transition(NetworkMessageTransition&,
                                       void* message); // complete433ACF75

struct NetworkMessageRuntime {
    engine::OwnerRegistry& callbacks; // canonical43475B94/B98/BC
    NetworkMessageTransition transition;
    const std::byte*& client_module; // 4346A55C
};
// Complete433877FF. Includes the transition, type23/type18 filters, the actual
// cancellation registry and the original channel vtable[-2]->original[40].
bool send_network_message(NetworkMessageRuntime&, void* channel, void* message,
                          std::uint8_t force_reliable, std::uint8_t voice);
void bind_network_message_runtime(NetworkMessageRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall send_network_message_hook(void* channel, void*, void* message,
                                          std::uint8_t force_reliable, std::uint8_t voice);
#else
bool send_network_message_hook(void* channel, void*, void* message, std::uint8_t force_reliable,
                               std::uint8_t voice);
#endif

// Complete43394E7C. Uses the same eight-line ring populated by43387EF4 and
// reads the canonical52EADF91 value on each frame. frame_time is globals+14.
void draw_console_output(ui::RenderQueue&, const ConsoleOutputConfig&, ColoredLogRing&,
                         float frame_time);
struct ConsoleOutputHandler : hud::HudFrameCallback {
    ConsoleOutputHandler(const ConsoleOutputConfig&, ColoredLogRing&,
                         const float& frame_time) noexcept;
    ConsoleOutputHandler(engine::OwnerRegistryBinding, const ConsoleOutputConfig&, ColoredLogRing&,
                         const float& frame_time);
    void initialize_vtable() noexcept;
    ConsoleOutputHandler(const ConsoleOutputHandler&) = delete;
    ConsoleOutputHandler& operator=(const ConsoleOutputHandler&) = delete;
    const ConsoleOutputConfig& config;
    ColoredLogRing& ring;
    const float& frame_time;
};
} // namespace skeet::misc
