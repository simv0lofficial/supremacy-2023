#pragma once
#include "common_controls_initialization.hpp"
#include "first_startup_services.hpp"
#include "jobs.hpp"

namespace skeet::engine {
// Genuine process imports borrowed from already-loaded Windows modules. The
// bundle resolves dependencies only; it constructs no owner, installs no hook
// and performs no startup callback or native data publication.
struct SourceInitializationImports {
    JobMemory memory;
    LoadedModuleEnvironment modules;
    NativeImageEnvironment images;
    ForwardedExportEnvironment forwarded_exports;
    NativeServiceTerminateProcess terminate_process;
    CommonControlsExports common_controls;

    FirstStartupServiceBindings first_startup_bindings(NativeDataImage&) const noexcept;
};

// Reuses the existing native process-heap, PE and loaded-module factories.
// Required modules/exports must already exist; this never loads a library.
SourceInitializationImports native_source_initialization_imports();
} //namespace skeet::engine
