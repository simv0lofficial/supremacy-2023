#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace skeet::misc {
// The supported VEngineCvar007 build's ConCommandBase prefix.
struct ConsoleCommandBase {
    void* vtable;
    ConsoleCommandBase* next;
    std::uint32_t registered;
    const char* name;
    const char* help;
    std::uint32_t flags;
};

class HiddenCvars {
public:
    HiddenCvars() = default;
    HiddenCvars(const HiddenCvars&) = delete;
    HiddenCvars& operator=(const HiddenCvars&) = delete;
    void* cvar = nullptr;
    ~HiddenCvars();
    void set_enabled(bool enabled);
    void update(ConsoleCommandBase* first, bool enabled);
private:
    struct RemovedFlags {
        std::string name;
        std::uint32_t flags;
    };
    std::unordered_map<ConsoleCommandBase*, RemovedFlags> removed_;
    bool enabled_ = false;
};
}
