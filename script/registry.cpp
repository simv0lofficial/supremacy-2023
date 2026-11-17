#include "skeet/script/registry.hpp"
#include "skeet/script/client_api.hpp"
#include "skeet/script/config_api.hpp"
#include "skeet/script/cvar_api.hpp"
#include "skeet/script/database.hpp"
#include "skeet/script/entity_api.hpp"
#include "skeet/script/globals_api.hpp"
#include "skeet/script/panorama_api.hpp"
#include "skeet/script/material_api.hpp"
#include "skeet/script/plist_api.hpp"
#include "skeet/script/renderer_api.hpp"
#include "skeet/script/ui_module.hpp"
#include "skeet/script/files.hpp"
#include "skeet/script/startup_materials.hpp"
#include <cstring>
#include <stdexcept>

namespace skeet::script {
namespace {
std::uint32_t extension_vtable(RegisteredExtension id) {
    switch (id) {
    case RegisteredExtension::materials:
        return 0x4346fe7c;
    case RegisteredExtension::no_op_434792f0:
        return 0x434721a4;
    case RegisteredExtension::no_op_4347935c:
    case RegisteredExtension::no_op_434792f4:
    case RegisteredExtension::no_op_4347965c:
    case RegisteredExtension::no_op_434792fc:
        return 0x4346fae4;
    case RegisteredExtension::no_op_434792f8:
        return 0x4346f618;
    case RegisteredExtension::ui:
        return 0x4346fda0;
    case RegisteredExtension::browser:
        return 0x434721b4;
    case RegisteredExtension::flags:
        return 0x4346f3f8;
    case RegisteredExtension::callbacks:
        return 0x4346fa44;
    case RegisteredExtension::files:
        return 0x43471da0;
    }
    throw std::invalid_argument("Unknown native script extension identity");
}
void free_extension_allocation(engine::JobMemory memory, void* allocation) {
    if (!allocation)
        return;
    auto* bytes = static_cast<std::byte*>(allocation);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}

ExtensionRegistry::ExtensionRegistry(ExtensionProcessBindings process)
    : process_(process), objects_{{{0, RegisteredExtension::materials},
                                   {0, RegisteredExtension::no_op_434792f0},
                                   {0, RegisteredExtension::no_op_4347935c},
                                   {0, RegisteredExtension::no_op_434792f4},
                                   {0, RegisteredExtension::ui},
                                   {0, RegisteredExtension::no_op_4347965c},
                                   {0, RegisteredExtension::no_op_434792f8},
                                   {0, RegisteredExtension::no_op_434792fc},
                                   {0, RegisteredExtension::browser},
                                   {0, RegisteredExtension::flags},
                                   {0, RegisteredExtension::callbacks},
                                   {0, RegisteredExtension::files}}} {
    if (!process_.memory.allocate || !process_.memory.free)
        throw std::invalid_argument(
            "Script extension constructors require their process heap imports");
    process_.materials.references = {0, 0, 0}; //native BSS before4347D8FD
}

ExtensionRegistry::~ExtensionRegistry() {
    free_extension_allocation(process_.memory, registration_.begin);
}

ExtensionObject& ExtensionRegistry::mutable_object(RegisteredExtension id) {
    for (auto& candidate : objects_)
        if (candidate.identity == id)
            return candidate;
    throw std::invalid_argument("Unknown native script extension identity");
}

const ExtensionObject& ExtensionRegistry::object(RegisteredExtension id) const {
    for (const auto& candidate : objects_)
        if (candidate.identity == id)
            return candidate;
    throw std::invalid_argument("Unknown native script extension identity");
}

void ExtensionRegistry::construct_extension(RegisteredExtension id) {
    auto& entry = mutable_object(id);
    void* pointer = &entry;
    engine::append_owner(registration_, &pointer, process_.memory); //434866CD ->433F1665
    if (id == RegisteredExtension::materials)
        initialize_material_extension_state(process_.materials); //three stores precede vtable
    entry.vtable = extension_vtable(id);
    if (id == RegisteredExtension::callbacks)
        for (auto& reason : process_.bridge.aim_miss_reasons)
            reason.clear();
    else if (id == RegisteredExtension::no_op_4347965c)
        auxiliary_ = {nullptr, nullptr, nullptr};
    if (shutdown_view_)
        refresh_shutdown_extensions();
}

void ExtensionRegistry::finalize_extension(RegisteredExtension id) {
    mutable_object(id).vtable = extension_vtable(id);
    if (id == RegisteredExtension::callbacks) {
        auto& reasons = process_.bridge.aim_miss_reasons;
        for (auto index = reasons.size(); index != 0; --index)
            std::string{}.swap(reasons[index - 1]);
    } else if (id == RegisteredExtension::no_op_4347965c) {
        free_extension_allocation(process_.memory, auxiliary_.begin);
    }
}

std::vector<RegisteredExtension> ExtensionRegistry::registered_order() const {
    std::vector<RegisteredExtension> result;
    const auto objects = registration_.entries();
    result.reserve(objects.size());
    for (const auto* pointer : objects)
        result.push_back(static_cast<const ExtensionObject*>(pointer)->identity);
    return result;
}

std::span<const RegisteredExtension> native_extension_order() noexcept {
    static constexpr RegisteredExtension order[]{
        RegisteredExtension::flags, //4334B57F ->43486734
        RegisteredExtension::no_op_434792f0, //4337EE46 ->4347DB37
        RegisteredExtension::no_op_434792f4, //433FCAFC ->4347F52B
        RegisteredExtension::no_op_434792f8, //433A13D2 ->434820A5
        RegisteredExtension::no_op_434792fc, //433FD92F ->4348492A
        RegisteredExtension::callbacks, //433DF100 ->4348DAE4
        RegisteredExtension::files, //43396E08 ->4348E4A2
        RegisteredExtension::no_op_4347935c, //433654BD ->4347DC88
        RegisteredExtension::browser, //43368710 ->43485D9F
        RegisteredExtension::materials, //433E42AC ->4347D8FD
        RegisteredExtension::no_op_4347965c, //433CC92F ->43481F09
        RegisteredExtension::ui, //433FC811 ->43480237
    };
    return order;
}

void ExtensionRegistry::register_exports(Bridge& bridge) const {
    if (!client || !config || !cvar || !database || !entity || !globals || !panorama ||
        !materialsystem || !plist || !renderer || !ui || !files)
        throw std::logic_error("Every native Lua module must have a concrete binding");
    for (const auto& module : builtin_registry()) {
        switch (module.kind) {
        case ModuleKind::client:
            client->register_exports(bridge);
            break;
        case ModuleKind::config:
            config->register_exports(bridge);
            break;
        case ModuleKind::cvar:
            cvar->register_exports(bridge);
            break;
        case ModuleKind::database:
            database->register_exports(bridge);
            break;
        case ModuleKind::entity:
            entity->register_exports(bridge);
            break;
        case ModuleKind::globals:
            globals->register_exports(bridge);
            break;
        case ModuleKind::panorama:
            panorama->register_exports(bridge);
            break;
        case ModuleKind::materialsystem:
            materialsystem->register_exports(bridge);
            break;
        case ModuleKind::plist:
            plist->register_exports(bridge);
            break;
        case ModuleKind::renderer:
            renderer->register_exports(bridge);
            break;
        case ModuleKind::ui:
            ui->register_exports();
            break;
        }
    }
    // Ten registered native extension objects use the real no-op 0x433C95B7.
    // Traverse the actual constructor registration order for the two concrete
    // bodies, after all eleven module binders have returned.
    for (const auto extension : registered_order()) {
        if (extension == RegisteredExtension::callbacks)
            callbacks.initialize(bridge);
        else if (extension == RegisteredExtension::files)
            files->register_exports(bridge);
    }
}

void ExtensionRegistry::refresh_shutdown_extensions() {
    if (!process_shutdown_ && (!client || !ui || !materialsystem || !panorama))
        throw std::logic_error("Lua shutdown requires the concrete bound module owners");
    const auto order = registered_order();
    shutdown_extensions_.clear();
    shutdown_extensions_.reserve(order.size());
    for (const auto extension : order) {
        switch (extension) {
        case RegisteredExtension::materials:
            shutdown_extensions_.emplace_back(
                process_shutdown_ ? &process_.materials : &materialsystem->shutdown_references());
            break;
        case RegisteredExtension::ui:
            shutdown_extensions_.emplace_back(process_shutdown_ ? &process_shutdown_->ui
                                                                : &ui->bindings());
            break;
        case RegisteredExtension::browser:
            shutdown_extensions_.emplace_back(process_shutdown_ ? &process_.browser
                                                                : &panorama->shutdown_state());
            break;
        case RegisteredExtension::flags:
            shutdown_extensions_.emplace_back(process_shutdown_ ? &process_shutdown_->flags
                                                                : &client->flags);
            break;
        case RegisteredExtension::callbacks:
            shutdown_extensions_.emplace_back(&callbacks);
            break;
        case RegisteredExtension::no_op_434792f0:
        case RegisteredExtension::no_op_4347935c:
        case RegisteredExtension::no_op_434792f4:
        case RegisteredExtension::no_op_4347965c:
        case RegisteredExtension::no_op_434792f8:
        case RegisteredExtension::no_op_434792fc:
        case RegisteredExtension::files:
            shutdown_extensions_.emplace_back(NoopExtension{});
            break;
        default:
            throw std::invalid_argument("Unknown registered native Lua extension");
        }
    }
    if (shutdown_view_) {
        shutdown_view_->extensions = shutdown_extensions_;
        shutdown_view_->ui = process_shutdown_ ? &process_shutdown_->ui : &ui->bindings();
    }
}

ShutdownState ExtensionRegistry::make_shutdown_state(Bridge& bridge,
                                                     std::span<const std::byte> config_bytes) {
    if (&bridge != &process_.bridge)
        throw std::invalid_argument("Lua shutdown requires the process extension bridge");
    if (shutdown_view_)
        throw std::logic_error("The process extension close view is already attached to its owner");
    refresh_shutdown_extensions();
    return {bridge, shutdown_extensions_,
            process_shutdown_ ? &process_shutdown_->ui : &ui->bindings(), config_bytes};
}

void ExtensionRegistry::attach_shutdown_state(ShutdownState& state) {
    if (&state.bridge != &process_.bridge)
        throw std::invalid_argument("Lua shutdown requires the process extension bridge");
    if (shutdown_view_ && shutdown_view_ != &state)
        throw std::logic_error("The process extension close view already belongs to another owner");
    refresh_shutdown_extensions();
    state.extensions = shutdown_extensions_;
    state.ui = process_shutdown_ ? &process_shutdown_->ui : &ui->bindings();
    shutdown_view_ = &state;
}
void ExtensionRegistry::attach_process_shutdown_state(ShutdownState& state,
                                                      ExtensionShutdownBindings containers) {
    if (process_shutdown_ || shutdown_view_)
        throw std::logic_error("The source process shutdown containers are already attached");
    if (&state.bridge != &process_.bridge || state.ui != &containers.ui)
        throw std::invalid_argument(
            "The source process shutdown state must borrow its actual bridge and UI");
    process_shutdown_.emplace(containers);
    attach_shutdown_state(state);
}

void ExtensionRegistry::detach_shutdown_state(const ShutdownState& state) noexcept {
    if (shutdown_view_ == &state)
        shutdown_view_ = nullptr;
}
} // namespace skeet::script
