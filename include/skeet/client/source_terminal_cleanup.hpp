#pragma once
#include <cstdint>
#include <span>

namespace skeet::script {
struct Bridge;
struct BrowserState;
struct ReferenceStrings;
class UiBindings;
class ScriptRuntime;
struct ScriptModulesBindings;
class SourceScriptRuntimeOwner;
}
namespace skeet::lagcomp {
class RecordPool;
struct HistoryRecordingBindings;
}
namespace skeet::client {
class NativeStartupSources;
class PlayerStateCache;
class PlayerRuntimeOwner;
class ClientRuntimeStorage;
struct ExitCallbacks;

struct SourceTerminalCleanupSlot {
    std::uint32_t operand, initializer, finalizer;
};
// Relative positions in the existing182-slot CRT plan. Execution remains
//interleaved with every other initializer; this is not a bulk cleanup pass.
std::span<const SourceTerminalCleanupSlot, 8> source_terminal_cleanup_plan() noexcept;

// Explicit source-owner profile. These are the containers used by the source
//producers, never casts of retained-image headers. Keep this context and all
//borrowed process owners alive through the same ExitCallbacks::run invocation.
class SourceTerminalCleanupBindings {
public:
    SourceTerminalCleanupBindings(ClientRuntimeStorage&, script::SourceScriptRuntimeOwner&,
                                  PlayerRuntimeOwner&);
    SourceTerminalCleanupBindings(ExitCallbacks&, script::Bridge&, script::BrowserState&,
                                  script::UiBindings&, script::ReferenceStrings&, PlayerStateCache&,
                                  lagcomp::RecordPool&);
    SourceTerminalCleanupBindings(const SourceTerminalCleanupBindings&) = delete;
    SourceTerminalCleanupBindings& operator=(const SourceTerminalCleanupBindings&) = delete;
    SourceTerminalCleanupBindings(SourceTerminalCleanupBindings&&) = delete;
    SourceTerminalCleanupBindings& operator=(SourceTerminalCleanupBindings&&) = delete;

    ExitCallbacks& callbacks;
    script::Bridge& bridge;
    script::BrowserState& browser;
    script::UiBindings& ui;
    script::ReferenceStrings& strings;
    PlayerStateCache& players;
    lagcomp::RecordPool& records;

    // Validate late factory aliases once engine/history/modules are composed.
    // The canonical owner constructor permits earlier CRT exit registration;
    // the explicit standalone constructor requires this provenance check.
    void require_producers(const script::ScriptRuntime&, const script::ScriptModulesBindings&,
                           const lagcomp::HistoryRecordingBindings&);
    void initialize(std::uint32_t native_initializer);
    void finalize(std::uint32_t native_finalizer);
    void (*source_finalizer(std::uint32_t native_finalizer))();

private:
    friend void bind_source_terminal_cleanup(NativeStartupSources&, SourceTerminalCleanupBindings&);
    bool producers_validated_ = false;
    bool source_owners_validated_ = false;
};

// Source bootstrap selects this profile before any CRT exit registration.
// It replaces only the eight source-owned CRT bodies; it must not be combined
//with executed native-image cleanup registrations for those same owners.
// NativeStartupSourceGraph selects it after its other binders. A staged source
// startup can select the already validated process-owner context before engine
// acquisition, then validate its late producer aliases after composition.
void bind_source_terminal_cleanup(NativeStartupSources&, SourceTerminalCleanupBindings&);
} //namespace skeet::client
