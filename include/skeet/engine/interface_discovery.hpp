#pragma once
#include "bindings.hpp"
#include "native_data_image.hpp"
#include "pe_exports.hpp"
#include <array>
#include <stdexcept>

namespace skeet::engine {
// Each role is backed by an exact captured CreateInterface registration. The
// interface objects remain borrowed engine objects; this bundle owns none.
struct CoreEngineInterfaces {
    void* engine_client;
    void* entity_list;
    void* base_client;
    void* prediction;
    void* surface;
    void* cvar;
    void* engine_trace;
};
struct CoreInterfaceCells {
    void*& engine_client; //4346A514
    void*& entity_list; //4346A890
    void*& base_client; //434683E0
    void*& prediction; //43469264
    void*& surface; //434689C0
    void*& cvar; //4346A87C
    void*& engine_trace; //43468304
};

// The working dump's real PE exports, linked registration names and creator
// results prove these pairs. This is a source acquisition manifest, not a
// claim about the unrecovered protected initializer's invocation order.
std::span<const InterfaceRequest, 7> core_interface_requests() noexcept;
CoreEngineInterfaces discover_core_interfaces(FactoryProvider&);
// All seven mandatory acquisitions finish before any canonical cell changes.
CoreEngineInterfaces discover_core_interfaces(FactoryProvider&, CoreInterfaceCells);
// Uses the existing provider, which resolves exports only in loaded modules.
CoreEngineInterfaces discover_core_interfaces(CoreInterfaceCells);

struct BuildInterfaceRecipe {
    std::uint32_t operand, module_cell, interface_hash, destination_cell, destination_xor;
    InterfaceRequest request;
};
enum class BuildInterfaceFailureStage {
    module_registry,
    registration,
    creator,
    factory
};
class BuildInterfaceDiscoveryFailure final : public std::runtime_error {
public:
    static constexpr std::int32_t native_exit_code = 23;
    BuildInterfaceDiscoveryFailure(const BuildInterfaceRecipe&, BuildInterfaceFailureStage);
    const std::uint32_t operand, destination_cell;
    const BuildInterfaceFailureStage stage;
};
struct BuildEngineInterfaces {
    std::array<void*, 41> objects{};
    void* at(std::string_view role) const;
    CoreEngineInterfaces core() const noexcept;
};

// Explicit data for this build's 41 acquisition triples. Their starting
// operands are 4C8..6A8; the final destination field is operand 6B0.
// ctx.h supplies the observed module-cell/hash/destination identifiers; exact
// names, creator results and plain-pointer destination representation are
// independently checked against the working module registration lists. No
// captured process pointer or erased operand-accessor implementation is used.
std::span<const BuildInterfaceRecipe, 41> build_interface_manifest() noexcept;

// Acquire in manifest order and publish each real result into its canonical
// source cell. Earlier published results survive an explicit source failure.
// Missing acquisitions raise BuildInterfaceDiscoveryFailure; the startup
// owner routes code 23 through its actual fail_initialization state/imports.
// The module-cell route uses the recovered registry/hash helpers and genuine
// external creators. FactoryProvider supplies a separate exact-name route.
BuildEngineInterfaces discover_build_interfaces(NativeDataImage&, NativeImageEnvironment);
BuildEngineInterfaces discover_build_interfaces(NativeDataImage&);
BuildEngineInterfaces discover_build_interfaces(FactoryProvider&, NativeDataImage&);
} //namespace skeet::engine
