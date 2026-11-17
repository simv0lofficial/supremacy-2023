#pragma once
#include "startup_convars.hpp"
#include <stdexcept>

namespace skeet::engine {
struct BuildStartupConVarSlot {
    std::uint32_t operand, address, name_hash, xor_key;
    const char* name;
};
// Explicit fixed-build data from the105 ordered B70..EB0 operand pairs.
// This manifest does not implement or claim the protected operand getter.
std::span<const BuildStartupConVarSlot, 105> build_startup_convar_slots() noexcept;

enum class BuildStartupConVarFailureReason {
    missing_candidate,
    candidate_is_command
};
class BuildStartupConVarFailure final : public std::runtime_error {
public:
    // Proven only for missing_candidate. The command-candidate continuation
    // reaches erased code4353F942 and has no recovered terminal policy.
    static constexpr std::int32_t native_missing_exit_code = 10;
    BuildStartupConVarFailure(std::size_t, const BuildStartupConVarSlot&,
                              BuildStartupConVarFailureReason);
    const std::size_t index;
    const std::uint32_t operand, address, name_hash;
    const char* const name;
    const BuildStartupConVarFailureReason reason;
};

class BuildStartupConVarProvider : public StartupConVarProvider {
public:
    virtual bool is_command(void* candidate) = 0;
};
class BuildStartupConVarBindings {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    NativeConVarReference at(std::uint32_t native_cell) const;
    StartupConVarBindings startup_bindings() const noexcept;
    client::StartupConVars startup_variables() const;
    NativeConVarReference accuracy_variable() const;

private:
    explicit BuildStartupConVarBindings(NativeDataImage& data) noexcept : data_(data) {}
    NativeDataImage& data_;
    friend BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage&, void*,
                                                                     BuildStartupConVarProvider&);
};

// Source bootstrap requires all105 genuine ConVars and publishes only after
// complete acquisition. The native success loop mutates each row separately.
// A missing candidate reaches4337F065/ZwTerminateThread(-2,10), retaining the
// prior stores; its failed row has no publication. The source transaction and
// typed failures remain explicit source policy. A command candidate's erased
// continuation4353F942 remains unresolved. Canonical XOR encodings are preserved.
BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage&, void* cvar,
                                                          BuildStartupConVarProvider&);
BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage&, void* cvar);
} //namespace skeet::engine
