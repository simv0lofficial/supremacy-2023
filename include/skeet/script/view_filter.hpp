#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/network/update_observers.hpp"
#include <array>

namespace skeet::script {

// Complete433A01A8. Every callback receives the same mutable table containing
// x/y/z/pitch/yaw/fov. Readback uses lua_tonumber after at least one callback
// succeeds, while the event is still marked active; roll has no table field.
int dispatch_override_view(Bridge&, std::array<float, 3>& origin, std::array<float, 3>& angles,
                           float& fov);

// Complete43325131. The callback edits copies; the recovered distance and
// finite/range gates determine which components reach the engine arguments.
void filter_script_view(Bridge&, float* origin, float* angles, float* fov);

// Parameterless methods on the same secondary interface. These events receive
// no arguments and borrow the same callback registry as override_view.
int dispatch_pre_render_3d(Bridge&); //4339DE92, slot0
int dispatch_post_render(Bridge&); //433AB83C, slot1
int dispatch_pre_render(Bridge&); //433EA04B, slot4

// Script-owner secondary interface at+38, native table4346F7F4. The endpoint
// borrows the canonical Lua bridge and registers in43475988/8C/90.
struct ScriptViewObserver {
    network::UpdateObserver endpoint;
    Bridge* bridge;
    ScriptViewObserver(Bridge&, engine::OwnerRegistryBinding updates);
};

} // namespace skeet::script
