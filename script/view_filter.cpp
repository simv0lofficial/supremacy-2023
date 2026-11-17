#include "skeet/script/view_filter.hpp"
#include <cmath>

namespace skeet::script {
namespace {
struct ViewArguments {
    std::array<float, 3>& origin;
    std::array<float, 3>& angles;
    float& fov;
};

void push_number_field(Bridge& bridge, const char* name, std::size_t length,
                       float value) { //4336B18E
    const auto& api = *bridge.api;
    api.pushlstring(bridge.state, name, length);
    api.pushnumber(bridge.state, static_cast<double>(value));
    api.settable(bridge.state, -3);
}

int marshal_view(Bridge& bridge, void* context) {
    const auto& view = *static_cast<const ViewArguments*>(context);
    bridge.api->createtable(bridge.state, 0, 6);
    push_number_field(bridge, "x", 1, view.origin[0]);
    push_number_field(bridge, "y", 1, view.origin[1]);
    push_number_field(bridge, "z", 1, view.origin[2]);
    push_number_field(bridge, "pitch", 5, view.angles[0]);
    push_number_field(bridge, "yaw", 3, view.angles[1]);
    push_number_field(bridge, "fov", 3, view.fov);
    return 1;
}

float number_field(Bridge& bridge, const char* name) { //433CC94A
    const auto& api = *bridge.api;
    api.getfield(bridge.state, -1, name);
    const auto value = static_cast<float>(api.tonumber(bridge.state, -1));
    api.settop(bridge.state, -2);
    return value;
}

void consume_view(Bridge& bridge, void* context, int completed) {
    if (completed <= 0)
        return;
    auto& view = *static_cast<ViewArguments*>(context);
    view.origin[0] = number_field(bridge, "x");
    view.origin[1] = number_field(bridge, "y");
    view.origin[2] = number_field(bridge, "z");
    view.angles[0] = number_field(bridge, "pitch");
    view.angles[1] = number_field(bridge, "yaw");
    view.fov = number_field(bridge, "fov");
}

int marshal_no_arguments(Bridge&, void*) {
    return 0;
} //43336942

float squared_change(const std::array<float, 3>& proposed, const float* current) {
    const float x = proposed[0] - current[0];
    const float y = proposed[1] - current[1];
    const float z = proposed[2] - current[2];
    // The native SSE path adds Y squared to X squared, then Z squared.
    return (y * y + x * x) + z * z;
}

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_SCRIPT_VIEW_MEMBER __thiscall
#define SKEET_SCRIPT_VIEW_FAST __fastcall
#else
#define SKEET_SCRIPT_VIEW_MEMBER
#define SKEET_SCRIPT_VIEW_FAST
#endif
void SKEET_SCRIPT_VIEW_MEMBER unchanged_phase(void*) {} //433CCE8E
void SKEET_SCRIPT_VIEW_MEMBER pre_render_3d_entry(void* receiver) {
    dispatch_pre_render_3d(*static_cast<ScriptViewObserver*>(receiver)->bridge);
}
void SKEET_SCRIPT_VIEW_MEMBER post_render_entry(void* receiver) {
    dispatch_post_render(*static_cast<ScriptViewObserver*>(receiver)->bridge);
}
void SKEET_SCRIPT_VIEW_MEMBER pre_render_entry(void* receiver) {
    dispatch_pre_render(*static_cast<ScriptViewObserver*>(receiver)->bridge);
}
void SKEET_SCRIPT_VIEW_FAST unchanged_player_view(void*, void*, std::uintptr_t, std::uintptr_t,
                                                  std::uintptr_t) {} //4337D653
void SKEET_SCRIPT_VIEW_FAST unchanged_post_view(void*, void*) {} //433CCE8E
void SKEET_SCRIPT_VIEW_FAST view_entry(void* receiver, float* origin, float* angles, float* fov) {
    filter_script_view(*static_cast<ScriptViewObserver*>(receiver)->bridge, origin, angles, fov);
}
const network::UpdateObserverVtable view_table{
    {pre_render_3d_entry, post_render_entry, unchanged_phase, unchanged_phase, pre_render_entry},
    unchanged_player_view,
    view_entry,
    unchanged_post_view};
#undef SKEET_SCRIPT_VIEW_MEMBER
#undef SKEET_SCRIPT_VIEW_FAST
}

int dispatch_override_view(Bridge& bridge, std::array<float, 3>& origin,
                           std::array<float, 3>& angles, float& fov) {
    if (!bridge.state)
        return 0;
    ViewArguments view{origin, angles, fov};
    return dispatch_callbacks(bridge, bridge.name(events::EventName::override_view).get(),
                              marshal_view, &view, consume_view);
}

int dispatch_pre_render_3d(Bridge& bridge) {
    return bridge.state
               ? dispatch_callbacks(bridge, bridge.name(events::EventName::pre_render_3d).get(),
                                    marshal_no_arguments, nullptr)
               : 0;
}

int dispatch_post_render(Bridge& bridge) {
    return bridge.state
               ? dispatch_callbacks(bridge, bridge.name(events::EventName::post_render).get(),
                                    marshal_no_arguments, nullptr)
               : 0;
}

int dispatch_pre_render(Bridge& bridge) {
    return bridge.state
               ? dispatch_callbacks(bridge, bridge.name(events::EventName::pre_render).get(),
                                    marshal_no_arguments, nullptr)
               : 0;
}

void filter_script_view(Bridge& bridge, float* origin, float* angles, float* fov) {
    std::array<float, 3> proposed_origin{origin[0], origin[1], origin[2]};
    std::array<float, 3> proposed_angles{angles[0], angles[1], angles[2]};
    auto proposed_fov = *fov;
    dispatch_override_view(bridge, proposed_origin, proposed_angles, proposed_fov);
    if (squared_change(proposed_origin, origin) > 0.1f) {
        for (std::size_t index = 0; index != proposed_origin.size(); ++index) {
            const auto value = proposed_origin[index];
            if (value != origin[index] && std::isfinite(value) && value > -16384.0f &&
                value < 16384.0f)
                origin[index] = value;
        }
    }
    if (squared_change(proposed_angles, angles) > 0.1f && std::isfinite(proposed_angles[0]) &&
        std::isfinite(proposed_angles[1]) && proposed_angles[0] > -360000.0f &&
        proposed_angles[0] < 360000.0f && proposed_angles[1] > -360000.0f &&
        proposed_angles[1] < 360000.0f && proposed_angles[2] > -360000.0f &&
        proposed_angles[2] < 360000.0f) {
        angles[0] = proposed_angles[0];
        angles[1] = proposed_angles[1];
    }
    if (proposed_fov != *fov && std::isfinite(proposed_fov) && proposed_fov > 0.0f &&
        proposed_fov < 180.0f)
        *fov = proposed_fov;
}

ScriptViewObserver::ScriptViewObserver(Bridge& owner, engine::OwnerRegistryBinding updates)
    : endpoint{nullptr}, bridge(&owner) {
    network::register_update_observer(updates, endpoint);
    endpoint.vtable = &view_table;
}

} // namespace skeet::script
