#pragma once
#include "skeet/engine/jobs.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include <span>

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;
struct NativePlayerStateLayout;
}
namespace skeet::lagcomp {
struct LagRecord;
}

namespace skeet::engine {
// Borrowed native layouts, not additional containers. Producers and these
// terminal destructors must address the same canonical x86 data cells.
struct StartupCallbackNode {
    std::uint32_t key;
    std::uint8_t value;
    std::array<std::byte, 3> padding;
    StartupCallbackNode* next;
};
struct StartupCallbackMap {
    StartupCallbackNode* first_bucket;
    StartupCallbackNode** buckets;
    std::uint32_t bucket_count, size;
};
template <class T> struct StartupVector {
    T* begin;
    T* end;
    T* limit;
};
template <class T> struct StartupInlineVector {
    T* begin;
    T* end;
    T* limit;
    std::uint32_t preserved;
    T* inline_buffer;
};
using StartupV8Slots = StartupVector<void**>;
struct StartupV8Handle {
    void** handle;
    std::uint8_t owns_handle;
    std::array<std::byte, 3> padding;
};
struct StartupUiBinding {
    void* widget;
    void* configuration;
    StartupVector<std::int32_t> references;
    std::uint32_t preserved;
    std::uint8_t borrowed;
    std::array<std::byte, 3> padding;
};
struct StartupWideText {
    char16_t* begin;
    char16_t* end;
    char16_t* limit;
    std::uint32_t preserved;
};
struct StartupReferenceString {
    std::int32_t reference;
    StartupWideText text;
    std::uint32_t color;
};
using DisposeStartupV8Global = void (*)(void**);

#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(StartupCallbackNode) == 12 && offsetof(StartupCallbackNode, next) == 8);
static_assert(sizeof(StartupCallbackMap) == 16 && offsetof(StartupCallbackMap, buckets) == 4);
static_assert(sizeof(StartupV8Slots) == 12 && sizeof(StartupV8Handle) == 8);
static_assert(sizeof(StartupUiBinding) == 28 && offsetof(StartupUiBinding, references) == 8);
static_assert(sizeof(StartupWideText) == 16 && sizeof(StartupReferenceString) == 24);
static_assert(offsetof(StartupReferenceString, text) == 4);
static_assert(sizeof(StartupInlineVector<std::uint32_t>) == 20);
#endif

// Complete ordinary helpers, including their deliberate dangling header
// fields. These are terminal operations and are not idempotent resets.
void cleanup_startup_callback_map(StartupCallbackMap&, JobMemory); //433CF5BD + bucket release
void cleanup_startup_v8_slots(StartupV8Slots&, DisposeStartupV8Global,
                              JobMemory); //433E223E + vector release
void cleanup_startup_v8_handle(StartupV8Handle&, DisposeStartupV8Global); //433F4D88
void cleanup_startup_ui_bindings(StartupVector<StartupUiBinding>&, JobMemory); //4337EA0A
void cleanup_startup_history(lagcomp::LagRecord*&, std::span<client::NativePlayerStateLayout*, 64>,
                             JobMemory); //4335BB22
void cleanup_startup_reference_strings(StartupInlineVector<StartupReferenceString>&,
                                       JobMemory); //43400D97
void cleanup_startup_inline_callbacks(StartupInlineVector<std::uint32_t>&, JobMemory); //433FBE15

class StartupVectorCleanupBindings {
public:
    StartupVectorCleanupBindings(PreservedInitialization&, client::ExitCallbacks&, JobMemory,
                                 DisposeStartupV8Global);
    StartupVectorCleanupBindings(std::shared_ptr<NativeDataImage>, client::ExitCallbacks&,
                                 JobMemory, DisposeStartupV8Global);
    StartupVectorCleanupBindings(const StartupVectorCleanupBindings&) = delete;
    StartupVectorCleanupBindings& operator=(const StartupVectorCleanupBindings&) = delete;
    StartupVectorCleanupBindings(StartupVectorCleanupBindings&&) = delete;
    StartupVectorCleanupBindings& operator=(StartupVectorCleanupBindings&&) = delete;

    std::shared_ptr<NativeDataImage> data;
    client::ExitCallbacks& callbacks;
    JobMemory memory;
    DisposeStartupV8Global dispose_global;
    void finalize(std::uint32_t native_address);
    void (*source_finalizer(std::uint32_t native_address))();
};

// The seven wrappers only append a finalizer through the existing exit
// registry. The two inline-vector constructors have separate field stores
// and remain the responsibility of the CRT initializer bindings.
void bind_startup_vector_cleanup(client::NativeStartupSources&, StartupVectorCleanupBindings&);
} // namespace skeet::engine
