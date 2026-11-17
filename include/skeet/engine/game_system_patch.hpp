#pragma once

namespace skeet::engine {
// Devirtualized from VM entry 0x4351CF96. The caller supplies the same
// target slot (native 0x434757CC) later consumed by GameSystemRuntime and unload.
// A null candidate leaves the previous slot untouched. No pointee is read.
// The native entry receives the candidate in EAX and returns success in AL;
// this ordinary C++ entry is for recovered callers, not a raw VM ABI detour.
bool capture_game_system_patch_target(void**& target_slot, void** candidate) noexcept;
}
