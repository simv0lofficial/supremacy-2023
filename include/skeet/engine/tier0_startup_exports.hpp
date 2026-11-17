#pragma once
#include "loaded_modules.hpp"
#include "native_data_image.hpp"
#include "pe_exports.hpp"

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using Tier0FloatTime = double(__cdecl*)(); //double in ST0
using Tier0MainThreadPredicate = bool(__cdecl*)(); //boolean in AL
#else
using Tier0FloatTime = double (*)();
using Tier0MainThreadPredicate = bool (*)();
#endif
struct Tier0StartupExports {
    Tier0FloatTime clock;
    void* allocator;
    Tier0MainThreadPredicate on_main_thread;
};

// Recovered three-row last-startup loop after tier0 lookup BBFFA7E0. Each real
// export lookup is followed immediately by its canonical store, in this order:
// Plat_FloatTime ->43468C84, g_pMemAlloc ->43467204, ThreadInMainThread ->43476920.
// After all three stores, dereference only the g_pMemAlloc export variable.
// The returned typed views contain those same final canonical values.
//
// Uses the original non-forwarding PE resolver4334AC0B, not an injected resolver
// callback. NativeImageEnvironment supplies its genuine RtlImageNtHeader import.
// An absent allocator export is an invalid native input (unguarded dereference);
// source reports that failure after the same preceding stores, instead of
// silently installing an empty allocator. No clock/allocator method is called.
Tier0StartupExports initialize_tier0_startup_exports(NativeDataImage&,
                                                     const LoadedModuleEnvironment&,
                                                     NativeImageEnvironment);
} // namespace skeet::engine
