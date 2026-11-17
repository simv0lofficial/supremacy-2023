#pragma once
#include "config.hpp"

namespace skeet::client {
class NativeStartupSources;
}
namespace skeet::anti_aim {
// Borrow the registered settings used by the menus and command owners. The
// binding object and these referents remain alive while startup can dispatch.
struct StartupInitializerBindings {
    ConfigRegistries& configuration;
    FakeLagSettings& fake_lag;
    OtherGlobalSettings& other;
};

// Bind four ordinary CRT initializer identities. Each callback initializes
// its own descriptor only; static metadata and the enabled byte survive.
// The startup driver's operand order selects when each callback runs.
void bind_native_startup_initializers(client::NativeStartupSources&, StartupInitializerBindings&);
} // namespace skeet::anti_aim
