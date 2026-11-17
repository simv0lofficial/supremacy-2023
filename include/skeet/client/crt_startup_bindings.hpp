#pragma once

namespace skeet::animation {
class FixAnimationBackupCleanupContext;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class NativeStartupSources;
class CrtRuntimeOwner;
class ConfigurationRuntimeOwner;
class StaticCrtRuntimeOwner;
class SourceTerminalCleanupBindings;

// Bind the complete182-slot CRT plan using only process storage. Engine and
// feature owners are acquired later by the supplied final43484F06 callback.
// All contexts must outlive the exit callbacks. This performs no initializer,
// engine acquisition or allocation of a feature owner; it checks the whole
// CRT plan before the scheduler is allowed to begin.
void bind_early_crt_sources(NativeStartupSources&, CrtRuntimeOwner&, ConfigurationRuntimeOwner&,
                            StaticCrtRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                            animation::FixAnimationBackupCleanupContext&,
                            SourceTerminalCleanupBindings&);
} //namespace skeet::client
