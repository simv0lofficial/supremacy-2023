#pragma once
#include "skeet/script/initialize.hpp"

namespace skeet::script::initialize_detail {
std::uintptr_t global_reference(void* state, const LuaVmLayout&);
ScriptRuntime* owner(void* state);
void bind(ScriptRuntime&);
void unbind(ScriptRuntime&);
}
