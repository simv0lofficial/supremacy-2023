#include "skeet/script/startup_initializers.hpp"
#include "skeet/script/registry.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/client/shutdown.hpp"
#include <stdexcept>

namespace skeet::script {
namespace {
StartupInitializerBindings* process_initializers = nullptr;

void finalize_flags() { //4333CE96 ->4338A7AB
    process_initializers->extensions.finalize_extension(RegisteredExtension::flags);
}
void finalize_no_op_434792f0() { //433881C8 ->4338371E
    process_initializers->extensions.finalize_extension(RegisteredExtension::no_op_434792f0);
}
void finalize_no_op_434792f4() { //433376B2 ->43336F77
    process_initializers->extensions.finalize_extension(RegisteredExtension::no_op_434792f4);
}
void finalize_no_op_434792f8() { //4336A832 ->433879E1
    process_initializers->extensions.finalize_extension(RegisteredExtension::no_op_434792f8);
}
void finalize_no_op_434792fc() { //4341D379 ->43336F77
    process_initializers->extensions.finalize_extension(RegisteredExtension::no_op_434792fc);
}
void finalize_callbacks() { //433374F9 ->433A0D0F
    process_initializers->extensions.finalize_extension(RegisteredExtension::callbacks);
}
void finalize_files() { //433B50F1 ->4340B002
    process_initializers->extensions.finalize_extension(RegisteredExtension::files);
}
void finalize_no_op_4347935c() { //4333F8F0 ->43336F77
    process_initializers->extensions.finalize_extension(RegisteredExtension::no_op_4347935c);
}
void finalize_browser() { //43406154 ->43400DFA
    process_initializers->extensions.finalize_extension(RegisteredExtension::browser);
}
void finalize_materials() { //4336EE05 ->4332C442
    process_initializers->extensions.finalize_extension(RegisteredExtension::materials);
}
void finalize_no_op_4347965c() { //4332AD92 ->433FBD87
    process_initializers->extensions.finalize_extension(RegisteredExtension::no_op_4347965c);
}
void finalize_ui() { //433B2632 ->433C95BF
    process_initializers->extensions.finalize_extension(RegisteredExtension::ui);
}

void initialize_flags(void* owner) { //4334B57F ->43486734, operand1D0
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::flags);
    (void)source.exit_callbacks.append(finalize_flags);
}
void initialize_no_op_434792f0(void* owner) { //4337EE46 ->4347DB37, operand1D4
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::no_op_434792f0);
    (void)source.exit_callbacks.append(finalize_no_op_434792f0);
}
void initialize_no_op_434792f4(void* owner) { //433FCAFC ->4347F52B, operand1D8
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::no_op_434792f4);
    (void)source.exit_callbacks.append(finalize_no_op_434792f4);
}
void initialize_no_op_434792f8(void* owner) { //433A13D2 ->434820A5, operand1DC
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::no_op_434792f8);
    (void)source.exit_callbacks.append(finalize_no_op_434792f8);
}
void initialize_no_op_434792fc(void* owner) { //433FD92F ->4348492A, operand1E4
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::no_op_434792fc);
    (void)source.exit_callbacks.append(finalize_no_op_434792fc);
}
void initialize_callbacks(void* owner) { //433DF100 ->4348DAE4, operand1F8
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::callbacks);
    (void)source.exit_callbacks.append(finalize_callbacks);
}
void initialize_files(void* owner) { //43396E08 ->4348E4A2, operand1FC
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::files);
    (void)source.exit_callbacks.append(finalize_files);
}
void initialize_no_op_4347935c(void* owner) { //433654BD ->4347DC88, operand200
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::no_op_4347935c);
    (void)source.exit_callbacks.append(finalize_no_op_4347935c);
}
void initialize_browser(void* owner) { //43368710 ->43485D9F, operand218
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::browser);
    (void)source.exit_callbacks.append(finalize_browser);
}
void initialize_materials(void* owner) { //433E42AC ->4347D8FD, operand21C
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::materials);
    (void)source.exit_callbacks.append(finalize_materials);
}
void initialize_no_op_4347965c(void* owner) { //433CC92F ->43481F09, operand220
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::no_op_4347965c);
    (void)source.exit_callbacks.append(finalize_no_op_4347965c);
}
void initialize_ui(void* owner) { //433FC811 ->43480237, operand234
    auto& source = *static_cast<StartupInitializerBindings*>(owner);
    source.extensions.construct_extension(RegisteredExtension::ui);
    (void)source.exit_callbacks.append(finalize_ui);
}
}

void bind_native_startup_initializers(client::NativeStartupSources& sources,
                                      StartupInitializerBindings& bindings) {
    if (process_initializers && process_initializers != &bindings)
        throw std::logic_error("Lua CRT exit callbacks already belong to another startup context");
    process_initializers = &bindings;
    sources.bind_initializer({0x4334B57FU, &bindings, initialize_flags});
    sources.bind_initializer({0x4337EE46U, &bindings, initialize_no_op_434792f0});
    sources.bind_initializer({0x433FCAFCU, &bindings, initialize_no_op_434792f4});
    sources.bind_initializer({0x433A13D2U, &bindings, initialize_no_op_434792f8});
    sources.bind_initializer({0x433FD92FU, &bindings, initialize_no_op_434792fc});
    sources.bind_initializer({0x433DF100U, &bindings, initialize_callbacks});
    sources.bind_initializer({0x43396E08U, &bindings, initialize_files});
    sources.bind_initializer({0x433654BDU, &bindings, initialize_no_op_4347935c});
    sources.bind_initializer({0x43368710U, &bindings, initialize_browser});
    sources.bind_initializer({0x433E42ACU, &bindings, initialize_materials});
    sources.bind_initializer({0x433CC92FU, &bindings, initialize_no_op_4347965c});
    sources.bind_initializer({0x433FC811U, &bindings, initialize_ui});
}
} // namespace skeet::script
