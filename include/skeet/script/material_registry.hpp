#pragma once
#include "skeet/script/bridge.hpp"
#include <span>

namespace skeet::script {

// The process C runtime owns these allocations. These are the four imported
// CRT entries used by 0x43486066 and 0x4334ED1D, without client callbacks.
struct MaterialRegistryImports {
    char* (*duplicate)(const char*) = nullptr;
    void* (*allocate)(std::size_t) = nullptr;
    void* (*copy)(void*, const void*, std::size_t) = nullptr;
    void (*release)(void*) = nullptr;
};
MaterialRegistryImports native_material_registry_imports();

struct MaterialRegistryEntry {
    char* name;
    LuaApi::CFunction function;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(MaterialRegistryEntry) == 8);
#endif

// Native process initializer 0x43486066 and destructor 0x4334ED1D. The owner
// must outlive every Lua state whose material metatable uses this registry.
// Construction only binds imports. The actual extension slot0 process pass
// invokes initialize_process() in its captured order before Lua registration.
// The table's existence, including an allocation failure, is the binder gate.
class MaterialMethodRegistry {
public:
    explicit MaterialMethodRegistry(MaterialRegistryImports);
    ~MaterialMethodRegistry();
    MaterialMethodRegistry(const MaterialMethodRegistry&) = delete;
    MaterialMethodRegistry& operator=(const MaterialMethodRegistry&) = delete;
    void initialize_process(); // 0x43486066
    void destroy_process(); // 0x4334ED1D, called by the process exit registry
    bool available() const noexcept {
        return table_ != nullptr;
    }
    std::span<const MaterialRegistryEntry> registrations() const noexcept;

private:
    MaterialRegistryImports imports_;
    MaterialRegistryEntry* table_ = nullptr; // 0x4347579C
};

} // namespace skeet::script
