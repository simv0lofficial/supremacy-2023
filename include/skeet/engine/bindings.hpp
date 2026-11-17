#pragma once
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include "skeet/engine/netvars.hpp"

namespace skeet::engine {

#if defined(_M_IX86) || defined(__i386__)
using InterfaceFactory = void*(__cdecl*)(const char*, int*);
#else
using InterfaceFactory = void* (*)(const char*, int*);
#endif

struct InterfaceRequest {
    std::string_view name;
    std::string_view module;
    std::string_view version;
};

struct BoundInterface {
    std::string name;
    std::string module;
    std::string version;
    void* object = nullptr;
};

class FactoryProvider {
public:
    virtual ~FactoryProvider() = default;
    virtual InterfaceFactory factory(std::string_view module) = 0;
};

// Resolves CreateInterface only in an already loaded, explicitly named module.
// There is no module loading, version probing, or alternate-name search.
class LoadedModuleFactories final : public FactoryProvider {
public:
    InterfaceFactory factory(std::string_view module) override;
};

// Integration adapter. The protected native factory manifest and its complete
// initialization sequence have not been recovered by this unit.
class InterfaceBindings {
public:
    // Every request is mandatory. A failed bind preserves the previous set.
    // Interface objects are borrowed from the engine and are never released.
    void bind(std::span<const InterfaceRequest> requests, FactoryProvider& provider);
    void clear() noexcept;
    void* at(std::string_view name) const;
    std::span<const BoundInterface> entries() const noexcept {
        return bindings_;
    }

private:
    std::vector<BoundInterface> bindings_;
};

// Known embedded interface consumers, with the base-client module corrected
// to client.dll by the working dump's registry and creator/object identity.
// This catalog is not the unrecovered native initialization manifest.
std::span<const InterfaceRequest> embedded_interface_consumers() noexcept;

// The existing protected operand accessor0x43493903 is a real external
// dependency. This interface exposes its values without replacing its logic.
class InitializationMetadata {
public:
    virtual ~InitializationMetadata() = default;
    virtual std::uintptr_t operand(std::uint32_t address) = 0;
};

struct DiscoveredDataMaps {
    DataMap* c69c5c29 = nullptr; // native global43477B50
    DataMap* a9cd7264 = nullptr; // native global43477B4C
};

// Complete native scheduler0x43407CDE. Recipe reads retain their native order;
// the operand accessor and loaded client module are supplied by initialization.
void initialize_native_netvars(void* base_client, std::byte* client_module,
                               InitializationMetadata& metadata, std::uint32_t*& cursor,
                               DiscoveredDataMaps& maps);

} // namespace skeet::engine
