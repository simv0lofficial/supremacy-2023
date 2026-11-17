#include "skeet/script/startup_materials.hpp"
#include "skeet/script/shutdown.hpp"

namespace skeet::script {
void initialize_material_extension_state(MaterialRefs& materials) noexcept {
    materials.references[0] = -1; //4347D907, global43479650
    materials.references[1] = -1; //4347D913, global43479654
    materials.references[2] = -1; //4347D91A, global43479658
}
} // namespace skeet::script
