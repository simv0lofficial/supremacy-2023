#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/client/command_phase_registry.hpp"

namespace skeet::script {

struct ScriptCommandOwnerBindings {
    client::ScriptCommand*& snapshot; //live43479300 slot, replaced by43486D1A
    const std::int32_t& choked_commands; //actual word reached through434757DC
    std::uint8_t& marshalling; //43475A5C, shared with the Lua client API
    client::ScriptCommandEnvironment environment;
    std::uint32_t& scheduler_flags; //434759C0, also used by CreateMove
};

// Complete normal command interface43470834, registered at native constructor
//4347FF6A. It borrows the main script owner's Bridge and the existing command
// snapshot pointer slot; process initialization can replace that allocation.
// It neither constructs another Lua state nor copies shared flags.
class ScriptCommandOwner final {
public:
    ScriptCommandOwner(Bridge&, ScriptCommandOwnerBindings,
                       client::CommandPhaseRegistry& actual_order);
    ScriptCommandOwner(const ScriptCommandOwner&) = delete;
    ScriptCommandOwner& operator=(const ScriptCommandOwner&) = delete;

    //4347FFF6 publishes after all whole-owner interfaces were registered.
    void publish_table(client::CommandPhaseRegistry&) noexcept;
    //433E41AD ->43347D5C: incoming[1] is the mutable move_yaw value.
    int phase2(client::Command&, std::array<float, 3>& incoming_angles, std::uint8_t& send_packet);
    //4334A2C8/4339F05D ->43331EB0. Each callback gets one shared table with
    // signed chokedcommands and command_number; its edits have no readback.
    int phase4(const client::CommandFrame&, const client::Command&);
    int phase6(const client::CommandFrame&, const client::Command&);

private:
    client::NativeCommandPhaseReceiver receiver_;
    Bridge& bridge_;
    ScriptCommandOwnerBindings bindings_;
};

} // namespace skeet::script
