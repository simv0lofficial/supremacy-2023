#pragma once
#include "loaded_modules.hpp"
#include "native_data_image.hpp"

namespace skeet::engine {
// Source integration for the observed engine64EE7E86/studiorender63DC118B
// builds. The PE identities and instruction fingerprints are build assumptions,
// separate from the recovered game-window and range-initializer algorithms.
// Reads already loaded module data/code only; calls no engine/studio function.
// Rejects unsupported identities, sections, signatures or pointer provenance
// before publishing any cell. Keep both modules loaded while consumers run.
//
// Writes43467288/43468B2C/43469288 and the two active range rows. Rows0/2 have
// their observed literal initializer1; their unused companion cells are left
// untouched. Call before owner4347D94F and initialize_module_ranges. This does
// not initialize the separately produced client-module cell4346A55C.
void initialize_build_window_bindings(NativeDataImage&, const LoadedModuleEnvironment&);
} // namespace skeet::engine
