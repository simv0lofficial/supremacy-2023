#pragma once

namespace skeet::script {
struct MaterialRefs;

// Material extension4347964C constructor4347D8FD: the common434866CD
// registration has already appended the object. Reset the three live Lua refs
// before the registry owner installs vtable4346FE7C. No refs are released here.
void initialize_material_extension_state(MaterialRefs&) noexcept;

// Process finalizer4336EE05->4332C442 only reinstalls vtable4346FE7C.
// Its vtable write belongs to ExtensionRegistry; it does not touch MaterialRefs.
} // namespace skeet::script
