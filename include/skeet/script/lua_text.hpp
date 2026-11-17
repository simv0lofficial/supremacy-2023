#pragma once
#include "skeet/script/bridge.hpp"

namespace skeet::script {
// Complete433FFE91. Capacity includes the trailing zero. Nil/boolean/numeric
// strings are concatenated without separators; light userdata is ignored.
// Successful __tostring calls leave their result on the Lua stack, as native.
// A failed conversion/overflow returns zero without terminating partial bytes.
std::size_t concatenate_lua(const LuaApi&, void* state, int first, char* output,
                            std::size_t capacity);
}
