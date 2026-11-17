#pragma once
#include "jobs.hpp"
#include "skeet/misc/native_text.hpp"
#include <cstdint>

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::ui {
class RendererDeviceListener;
class RendererContainer;
}

namespace skeet::engine {
struct UnloadState;

// These are borrowed native allocation headers, not std::vector layouts. The
// terminal helpers preserve begin/end/limit and the fourth word after freeing.
// They do not destroy the separate STL-backed preset/catalog/script objects;
// their native allocation producers require an explicit ownership migration.
template <class T> struct StartupOwnedVector {
    T* begin;
    T* end;
    T* limit;
    std::uint32_t preserved;
};
using StartupOwnedPointers = StartupOwnedVector<void*>;
using StartupOwnedTexts = StartupOwnedVector<misc::NativeText>;

// Only the prefix touched by4334D2C1 is interpreted. Source bytes are freed
// first, then the name. Neither field is cleared before freeing the object.
struct StartupModulePrefix {
    std::array<std::byte, 0xC> preserved;
    misc::NativeText name;
    void* source;
};
using StartupOwnedModules = StartupOwnedVector<StartupModulePrefix*>;
struct StartupTextBucketNode {
    misc::NativeText text;
    std::uint32_t preserved;
    StartupTextBucketNode* next;
};
struct StartupPointerBucketNode {
    std::array<std::uint32_t, 3> preserved;
    StartupPointerBucketNode* next;
};
template <class Node> struct StartupBucketTable {
    std::uintptr_t preserved;
    Node** buckets;
    std::uint32_t bucket_count;
    std::uint32_t size;
};
using StartupTextBuckets = StartupBucketTable<StartupTextBucketNode>;
using StartupPointerBuckets = StartupBucketTable<StartupPointerBucketNode>;
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(StartupOwnedPointers) == 0x10);
static_assert(sizeof(StartupOwnedTexts) == 0x10);
static_assert(sizeof(StartupOwnedModules) == 0x10);
static_assert(offsetof(StartupModulePrefix, name) == 0xC);
static_assert(offsetof(StartupModulePrefix, source) == 0x1C);
static_assert(offsetof(StartupTextBucketNode, next) == 0x14);
static_assert(offsetof(StartupPointerBucketNode, next) == 0xC);
static_assert(offsetof(StartupTextBuckets, buckets) == 4);
static_assert(offsetof(StartupTextBuckets, bucket_count) == 8);
static_assert(offsetof(StartupTextBuckets, size) == 0xC);
static_assert(sizeof(StartupTextBuckets) == 0x10);
static_assert(sizeof(StartupPointerBuckets) == 0x10);
#endif

void finalize_startup_owned_pointers(StartupOwnedPointers&, JobMemory); //43393934
void finalize_startup_owned_texts(StartupOwnedTexts&, JobMemory); //43383DAD
void finalize_startup_owned_modules(StartupOwnedModules&, JobMemory); //433C8710
void finalize_startup_text_buckets(StartupTextBuckets&, JobMemory); //4334320D
void finalize_startup_pointer_buckets(StartupPointerBuckets&, JobMemory); //433F0A02

struct StartupContainerCleanupBindings {
    client::ExitCallbacks& callbacks;
    JobMemory memory;
    StartupOwnedPointers& preset_hotkeys; //43478DA8
    StartupOwnedTexts& agent_paths_ct; //43479088
    StartupOwnedTexts& agent_paths_t; //43479078
    StartupOwnedModules& builtin_modules; //434792E0
    StartupOwnedModules& required_modules; //434792D0
    StartupTextBuckets& text_buckets; //4346D700
    StartupPointerBuckets& pointer_buckets; //4346DA40
    // This exact listener exists before renderer discovery and follows the
    // canonical nullable renderer slot. Registration shares UnloadState.
    ui::RendererDeviceListener& renderer_listener;
    UnloadState& unload;
};

// Keep this owner and all borrowed storage alive through ExitCallbacks::run().
// No-argument finalizers select this one process-lifetime source context.
class StartupContainerCleanup {
public:
    explicit StartupContainerCleanup(StartupContainerCleanupBindings);
    StartupContainerCleanup(const StartupContainerCleanup&) = delete;
    StartupContainerCleanup& operator=(const StartupContainerCleanup&) = delete;
    client::ExitCallbacks& exit_callbacks() const noexcept {
        return bindings_.callbacks;
    }
    void append_finalizer(std::uint32_t native_finalizer) const;
    void (*source_callback(std::uint32_t native_finalizer) const)();
    const StartupContainerCleanupBindings& bindings() const noexcept {
        return bindings_;
    }

private:
    StartupContainerCleanupBindings bindings_;
    static const StartupContainerCleanup& bound();
    static void preset_hotkeys(); //4333087C
    static void renderer_listener(); //433D2E2B ->433D0304
    static void agent_paths_ct(); //4331ECD4
    static void agent_paths_t(); //433F4BCB
    static void builtin_modules(); //43336C3C
    static void required_modules(); //4334D2AB
    static void text_buckets(); //4335851B
    static void pointer_buckets(); //433ACF65
};
void bind_startup_container_cleanup(client::NativeStartupSources&, StartupContainerCleanup&);
} //namespace skeet::engine
