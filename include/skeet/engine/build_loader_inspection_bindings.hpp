#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
// The supplied loader's four inputs consumed by the last startup VM. Resolve
// against actual canonical engine/material modules and VEngineClient014 cells.
// Supported builds: engine64EE7E86, materialsystem63DC3DE4. All code/section
// checks precede publication; no captured client pointer or RVA-only bind is used.
//
// 4346D4D0: engine virtual222 code (its LEA displacement is inspected).
// 4346D4C4: interior engine counter/arithmetic code.
// 4346D4C8: interior material-array call instruction.
// 4346D4C0: address of the engine tree header, whose value can legitimately0.
// These are readable instruction/data inputs, never executable callbacks.
void initialize_build_loader_inspection_inputs(NativeDataImage&);
} // namespace skeet::engine
