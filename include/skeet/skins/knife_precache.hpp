#pragma once
#include "skeet/skins/sequences.hpp"

namespace skeet::skins {
// Complete4338A892 and its433AE8B7 table lookup. string_tables is the actual
// network-string-table container43467198. Model strings and indices remain
// engine-owned; this only publishes the three resolved indices per knife.
void refresh_knife_precache(SequenceRuntime&, void* string_tables);
} // namespace skeet::skins
