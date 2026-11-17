#pragma once
#include <cstdint>

namespace skeet {
class PreservedConfigStorage;
}
namespace skeet::ui {
struct MainMenuConfigPathInitialization {
    std::uint32_t processor_signature;
    std::uint32_t filename_key;
    bool existed;
    bool converted;
};
// Produce the actual startup9.cfg path before RootMenu/CONFIG construction.
// The canonical NativeConfigPath receives the real RTL-owned counted name;
// existing ShutdownRuntime releases it after hook/menu lifetimes finish.
MainMenuConfigPathInitialization initialize_main_menu_config_path(PreservedConfigStorage&);
}
