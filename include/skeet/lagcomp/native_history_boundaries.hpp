#pragma once
#include "history_animation_publish.hpp"
#include "history_environment.hpp"
#include "history_yaw_side.hpp"
#include "skeet/animation/entity_update.hpp"

namespace skeet::lagcomp {
struct HistoryNumericBindings {
    HistoryYawSideFmod fmod;
    HistoryEnvironmentAtan2 environment_atan2;
    double (*atan2)(double, double);
    float (*power)(float, float);
    float (*arccosine)(float);
    int (*contains_sequence_name)(const char*, const char*);
};
// Adapters for the genuine loaded msvcrt imports. No replacement library or
// numeric implementation is selected. Missing exports are explicit errors.
HistoryNumericBindings native_history_numeric_bindings();
// The actual engine movement entry consumes ECX/EDX plus two caller-cleaned
// stack arguments. The animation entry consumes ECX/EDX, XMM0/1/2 and RET4.
// These functions adapt only those external ABIs; the recovered source bodies
// retain every displacement/animation decision and their optional-entry gates.
HistoryAnimationMovementEngine native_history_movement_engine(void* entry);
animation::AnimationUpdateEngine native_history_animation_engine(void* entry);
} //namespace skeet::lagcomp
