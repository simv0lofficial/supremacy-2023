#pragma once
#include "bridge.hpp"
#include "shutdown.hpp"
#include "skeet/engine/owner_registry.hpp"
#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace skeet::script {
enum class ModuleKind : std::uint8_t {
    client,
    config,
    cvar,
    database,
    entity,
    globals,
    panorama,
    materialsystem,
    plist,
    renderer,
    ui,
};
struct ExportDefinition {
    const char* name;
    std::uint32_t original_entry;
};
struct ModuleDefinition {
    ModuleKind kind;
    const char* name;
    std::uint32_t original_binder;
    std::span<const ExportDefinition> exports;
};
// The recovered, readable 11-module / 139-function catalog from 0x433ED18D.
std::span<const ModuleDefinition> builtin_registry() noexcept;

inline constexpr std::size_t native_registry_size = 0xcfc;
inline constexpr std::uint32_t native_registry_begin = 0x43474a54;
// Exact 0x433ED18D word decoder. Word zero is untouched. Native allocation uses
// HEAP_ZERO_MEMORY; callers that compare raw buffers should initialize it too.
void decode_native_registry(std::span<const std::byte> encoded, std::span<std::byte> decoded,
                            std::uint32_t relocated_begin);
struct DecodedExport {
    std::string name;
    std::uint32_t original_entry;
};
struct DecodedModule {
    ModuleKind kind;
    std::string name;
    std::uint32_t original_binder;
    std::vector<DecodedExport> exports;
};
std::vector<DecodedModule> inspect_native_registry(std::span<const std::byte> decoded,
                                                   std::uint32_t relocated_begin);

class ClientModule;
class ConfigModule;
class CvarModule;
class DatabaseModule;
class EntityModule;
class GlobalsModule;
class PanoramaModule;
class MaterialModule;
class PlistModule;
class RendererModule;
class UiModule;
class FileModule;

// Identities appended by the twelve concrete native global constructors.
// The captured 0x43475944..48 sequence supplies order; pointer address order
// is not used to guess the order of C++ static initialization.
enum class RegisteredExtension : std::uint32_t {
    materials = 0x4347964c,
    no_op_434792f0 = 0x434792f0,
    no_op_4347935c = 0x4347935c,
    no_op_434792f4 = 0x434792f4,
    ui = 0x43479780,
    no_op_4347965c = 0x4347965c,
    no_op_434792f8 = 0x434792f8,
    no_op_434792fc = 0x434792fc,
    browser = 0x43479608,
    flags = 0x43479118,
    callbacks = 0x43479308,
    files = 0x43479304,
};
// Relative order from the supplied startup context operands1D0..234 and the
// twelve ordinary wrapper/constructor pairs, rather than address sorting.
std::span<const RegisteredExtension> native_extension_order() noexcept;

struct ExtensionProcessBindings {
    Bridge& bridge;
    MaterialRefs& materials;
    BrowserState& browser;
    engine::JobMemory memory;
};
struct ExtensionShutdownBindings {
    UiBindings& ui;
    ReferenceStrings& flags;
};
struct ExtensionObject {
    std::uint32_t vtable = 0;
    RegisteredExtension identity;
};
struct ExtensionAuxiliaryVector {
    void* begin = nullptr;
    void* end = nullptr;
    void* limit = nullptr;
};

// Concrete typed module initializers, in the decoded native constructor order.
// Every required module must be bound before any Lua registration is performed.
class ExtensionRegistry {
public:
    explicit ExtensionRegistry(ExtensionProcessBindings);
    ~ExtensionRegistry();
    ExtensionRegistry(const ExtensionRegistry&) = delete;
    ExtensionRegistry& operator=(const ExtensionRegistry&) = delete;
    void construct_extension(RegisteredExtension); //434866CD then native payload/vtable writes
    void finalize_extension(RegisteredExtension);
    const ExtensionObject& object(RegisteredExtension) const;
    ExtensionAuxiliaryVector& auxiliary_vector() noexcept {
        return auxiliary_;
    }
    Bridge& process_bridge() noexcept {
        return process_.bridge;
    }
    MaterialRefs& material_references() noexcept {
        return process_.materials;
    }
    BrowserState& browser_state() noexcept {
        return process_.browser;
    }
    ClientModule* client = nullptr;
    ConfigModule* config = nullptr;
    CvarModule* cvar = nullptr;
    DatabaseModule* database = nullptr;
    EntityModule* entity = nullptr;
    GlobalsModule* globals = nullptr;
    PanoramaModule* panorama = nullptr;
    MaterialModule* materialsystem = nullptr;
    PlistModule* plist = nullptr;
    RendererModule* renderer = nullptr;
    UiModule* ui = nullptr;
    CallbackState callbacks;
    FileModule* files = nullptr;
    void register_exports(Bridge&) const;
    std::vector<RegisteredExtension> registered_order() const;
    // The process constructor vector can grow before or after ScriptOwner is
    // constructed. The attached state's close view follows those real appends.
    ShutdownState make_shutdown_state(Bridge&, std::span<const std::byte> current_config = {});
    void attach_shutdown_state(ShutdownState&);
    // Production process containers exist before the late ScriptOwner. CRT
    // extension constructors update this same close view without modules.
    void attach_process_shutdown_state(ShutdownState&, ExtensionShutdownBindings);
    bool is_process_shutdown_state(const ShutdownState& state) const noexcept {
        return process_shutdown_.has_value() && shutdown_view_ == &state;
    }
    void detach_shutdown_state(const ShutdownState&) noexcept;

private:
    ExtensionProcessBindings process_;
    std::array<ExtensionObject, 12> objects_;
    engine::OwnerRegistry registration_;
    ExtensionAuxiliaryVector auxiliary_;
    std::vector<CloseExtension> shutdown_extensions_;
    ShutdownState* shutdown_view_ = nullptr;
    std::optional<ExtensionShutdownBindings> process_shutdown_;
    ExtensionObject& mutable_object(RegisteredExtension);
    void refresh_shutdown_extensions();
};
} // namespace skeet::script
