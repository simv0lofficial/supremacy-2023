#pragma once

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::script {
class ExtensionRegistry;

// Process storage, shared with the script primary owner and final shutdown.
// The context remains alive until the last registered exit callback returns.
struct StartupInitializerBindings {
    ExtensionRegistry& extensions;
    client::ExitCallbacks& exit_callbacks;
};

// The startup operand scheduler selects each of the twelve ordinary CRT
// wrappers separately. Binding these calls does not construct an extension.
void bind_native_startup_initializers(client::NativeStartupSources&, StartupInitializerBindings&);
} // namespace skeet::script
