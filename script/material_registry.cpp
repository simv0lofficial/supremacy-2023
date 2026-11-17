#include "skeet/script/material_methods.hpp"
#include "skeet/script/material_registry.hpp"
#include <array>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace skeet::script {

std::span<const MaterialMethodRegistration> MaterialMethods::registrations() noexcept {
    static constexpr std::array<MaterialMethodRegistration, 8> methods{{
        {"get_name", get_name_entry, 0x43408733},
        {"reload", reload_entry, 0x433D2DE4},
        {"alpha_modulate", alpha_modulate_entry, 0x433D195E},
        {"color_modulate", color_modulate_entry, 0x433982F3},
        {"set_shader_param", set_shader_param_entry, 0x4337F6D1},
        {"get_shader_param", get_shader_param_entry, 0x4339CA67},
        {"set_material_var_flag", set_material_var_flag_entry, 0x43328673},
        {"get_material_var_flag", get_material_var_flag_entry, 0x433E9AEF},
    }};
    return methods;
}

MaterialRegistryImports native_material_registry_imports() {
    return {&_strdup, &std::malloc, &std::memcpy, &std::free};
}

MaterialMethodRegistry::MaterialMethodRegistry(MaterialRegistryImports imports)
    : imports_(imports) {
    if (!imports_.duplicate || !imports_.allocate || !imports_.copy || !imports_.release)
        throw std::invalid_argument("Material registry requires its process CRT imports");
}

void MaterialMethodRegistry::initialize_process() { // 0x43486066
    std::array<MaterialRegistryEntry, 9> entries{};
    std::size_t index = 0;
    for (const auto& method : MaterialMethods::registrations())
        entries[index++] = {imports_.duplicate(method.name), method.function};
    table_ = static_cast<MaterialRegistryEntry*>(imports_.allocate(sizeof(entries)));
    if (table_)
        imports_.copy(table_, entries.data(), sizeof(entries));
    // The native initializer does not release any duplicated names when the
    // final table allocation fails; its process destructor is not registered.
}

MaterialMethodRegistry::~MaterialMethodRegistry() {
    destroy_process();
}

void MaterialMethodRegistry::destroy_process() { // 0x4334ED1D
    if (!table_)
        return;
    for (auto* entry = table_; entry->name; ++entry)
        imports_.release(entry->name);
    imports_.release(table_);
    table_ = nullptr; // The explicit exit pass can precede the C++ owner release.
}

std::span<const MaterialRegistryEntry> MaterialMethodRegistry::registrations() const noexcept {
    if (!table_)
        return {};
    const auto* end = table_;
    while (end->name)
        ++end;
    return {table_, static_cast<std::size_t>(end - table_)};
}

} // namespace skeet::script
