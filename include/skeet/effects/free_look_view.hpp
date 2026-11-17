#pragma once
#include "skeet/client/commands.hpp"
#include "skeet/network/update_observers.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::effects {

// The view half follows the existing command state in the same native owner.
// 433CD16F captures the origin; 433CD17C stamps the current engine frame.
struct FreeLookViewCapture {
    std::array<float, 3> origin;
    std::int32_t frame;
};
struct FreeLookOwnerState {
    client::FreeLookState command;
    FreeLookViewCapture view;
};
static_assert(offsetof(FreeLookOwnerState, view) == 0x28);
static_assert(offsetof(FreeLookOwnerState, view) + offsetof(FreeLookViewCapture, frame) == 0x34);
static_assert(sizeof(FreeLookOwnerState) == 0x38);

struct FreeLookViewBindings {
    void* const& input; //4346A894; slots32/34
    void* const& prediction; //43469264; temporary byte+8
    void* const& local_player; //43475808; captured once after the key gate
    std::uint32_t life_state; //decoded9D920E35
    const std::int32_t& frame; //*434678F8
    engine::NativeConVarReference camera_distance; //43467AA0 ^8985EFCE, live encoded cell
    engine::NativeConVarReference camera_mode; //4346A7DC ^B2005B9C, live encoded cell
    void* const& engine_trace; //43468304; actual TraceRay slot5
    void* const* trace_filter_vtable; //genuine external434758B4 table
    const HotkeyBinding& free_look_key; //43468274/43468288
    const std::uint8_t& third_person; //43468A10
    const HotkeyBinding& third_person_key; //43468448/4346845C
    InputBitmap keys;
    RageTrigonometry trigonometry; //actual imported SSE sine/cosine
};

class FreeLookViewRuntime {
public:
    client::FreeLookState& state;
    FreeLookViewCapture& capture;
    FreeLookViewBindings& bindings;

    FreeLookViewRuntime(client::FreeLookState& state, FreeLookViewCapture& capture,
                        FreeLookViewBindings& bindings)
        : state(state), capture(capture), bindings(bindings) {}
    void reset() noexcept; //complete4341781B; also the failed43336F85 gate
    bool third_person_active(); //complete433FBBCD, including the second hotkey
    // Complete43336F85. Both camera queries and the repeated third-person test
    // retain their native order; player slots212/278 and TraceRay are real calls.
    void update(float* origin, float* angles);
};

// Actual secondary interface4347269C. Slot6 is the concrete433B1E73 wrapper;
// all remaining entries are its original no-op methods, with their real ABIs.
struct FreeLookViewObserver {
    network::UpdateObserver endpoint;
    FreeLookViewRuntime* runtime;
    explicit FreeLookViewObserver(FreeLookViewRuntime&, bool initialize_table = true);
    void initialize_vtable() noexcept;
};

} //namespace skeet::effects
