#pragma once
#include <cstdint>

namespace skeet::ui {
class IconCache;

struct StartupIcons {
    std::int32_t& grenade; //43468E3C
    std::int32_t& inferno; //4346842C
    std::int32_t& bomb; //43467DA8
};

// Complete433283A5. Uses the cache belonging to renderer4347690C.
void initialize_startup_icons(IconCache& cache, StartupIcons& icons);
}
