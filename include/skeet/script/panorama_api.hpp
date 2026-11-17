#pragma once
#include "skeet/script/bridge.hpp"
#include "skeet/script/shutdown.hpp"
#include <array>
#include <span>

namespace skeet::script {

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_V8_METHOD __thiscall
#define SKEET_V8_OS __stdcall
#else
#define SKEET_V8_METHOD
#define SKEET_V8_OS
#endif

// V8 5.8 Local/MaybeLocal both contain a pointer to one tagged-object slot.
// The slots belong to the engine isolate; Lua userdata never owns a Local.
struct V8Local {
    void** handle = nullptr;
};
struct V8MaybeBoolean {
    bool has_value = false, value = false;
};
static_assert(sizeof(V8Local) == sizeof(void*));
static_assert(sizeof(V8MaybeBoolean) == 2);

// Semantic views of the v8.dll APIs. The import binder supplies the explicit
// MSVC x86 result buffers for Local/MaybeLocal/Maybe return values.
// Scope/TryCatch/Utf8Value storage is constructed and destroyed by that DLL.
struct PanoramaV8Api {
    void(SKEET_V8_METHOD* isolate_enter)(void*);
    void(SKEET_V8_METHOD* isolate_exit)(void*);
    void(SKEET_V8_METHOD* handle_scope_construct)(void*, void*);
    void(SKEET_V8_METHOD* handle_scope_destroy)(void*);
    void(SKEET_V8_METHOD* try_catch_construct)(void*, void*);
    void(SKEET_V8_METHOD* try_catch_destroy)(void*);
    bool(SKEET_V8_METHOD* has_caught)(const void*);
    V8Local(SKEET_V8_METHOD* exception)(const void*);
    void** (*create_handle)(void*, void*);
    void** (*globalize_reference)(void*, void**);
    void (*dispose_global)(void**);
    void(SKEET_V8_METHOD* context_enter)(void**);
    void(SKEET_V8_METHOD* context_exit)(void**);
    V8Local(SKEET_V8_METHOD* context_global)(void**);
    V8Local (*string_new)(void*, const char*, int, int); // Local overload
    V8Local (*string_maybe_new)(void*, const char*, int, int);
    V8Local (*number_new)(void*, double);
    V8Local (*object_new)(void*);
    V8Local (*array_new)(void*, int);
    V8Local (*compile_unbound_script)(void*, void*, int);
    V8Local(SKEET_V8_METHOD* bind_to_current_context)(void**);
    V8Local(SKEET_V8_METHOD* script_run)(void**, V8Local);
    V8Local(SKEET_V8_METHOD* object_get)(void**, V8Local, V8Local);
    bool(SKEET_V8_METHOD* object_set)(void**, V8Local, V8Local);
    V8MaybeBoolean(SKEET_V8_METHOD* array_set)(void**, V8Local, std::uint32_t, V8Local);
    V8Local(SKEET_V8_METHOD* array_get)(void**, V8Local, std::uint32_t);
    std::uint32_t(SKEET_V8_METHOD* array_length)(void**);
    V8Local(SKEET_V8_METHOD* function_call)(void**, V8Local, V8Local, int, V8Local*);
    V8Local(SKEET_V8_METHOD* object_call)(void**, V8Local, V8Local, int, V8Local*);
    bool(SKEET_V8_METHOD* is_function)(void**);
    bool(SKEET_V8_METHOD* is_object)(void**);
    bool(SKEET_V8_METHOD* is_array)(void**);
    bool(SKEET_V8_METHOD* is_callable)(void**);
    bool(SKEET_V8_METHOD* is_boolean)(void**);
    bool(SKEET_V8_METHOD* boolean_value)(void**);
    bool(SKEET_V8_METHOD* is_int32)(void**);
    bool(SKEET_V8_METHOD* is_uint32)(void**);
    bool(SKEET_V8_METHOD* is_number)(void**);
    V8Local(SKEET_V8_METHOD* to_int32)(void**, V8Local);
    V8Local(SKEET_V8_METHOD* to_uint32)(void**, V8Local);
    V8Local(SKEET_V8_METHOD* to_number)(void**, V8Local);
    V8Local(SKEET_V8_METHOD* to_object)(void**, V8Local);
    V8Local(SKEET_V8_METHOD* to_string)(void**, V8Local);
    std::int32_t(SKEET_V8_METHOD* int32_value)(void**);
    std::uint32_t(SKEET_V8_METHOD* uint32_value)(void**);
    double(SKEET_V8_METHOD* number_value)(void**);
    void(SKEET_V8_METHOD* utf8_construct)(void*, V8Local);
    void(SKEET_V8_METHOD* utf8_destroy)(void*);
    void(SKEET_V8_METHOD* cached_data_destroy)(void*);
    // Native CachedData wrapper allocation uses the current process heap.
    void* process_heap;
    unsigned char(SKEET_V8_OS* heap_free)(void*, unsigned long, void*);
};

// Bind exact decorated exports from already loaded v8.dll. No DLL is loaded
// and a missing export is an initialization error.
PanoramaV8Api bind_loaded_panorama_v8_api();

using PanoramaBeginEngineCall = void(SKEET_V8_OS*)(void*);
using PanoramaEndEngineCall = void (*)();
struct PanoramaServices {
    void* engine_service = nullptr; // native 0x43468E98
    const std::uint8_t* unsafe_43475798 = nullptr; // ScriptsMenuValues::allow_unsafe
    const std::uint8_t* unsafe_43475a5c = nullptr; // ScriptCommandOwnerBindings::marshalling
    const bool* unsafe_4347582c = nullptr;
    PanoramaV8Api v8{};
    // Native panorama.dll boundaries 0x5BCBDFE0 / 0x5BCBE020, stored in
    // 0x4346D628 / 0x4346D62C. These surround only a JavaScript function call.
    PanoramaBeginEngineCall begin_engine_call = nullptr;
    PanoramaEndEngineCall end_engine_call = nullptr;
};

// One owner is retained by the Lua closures until its state closes.
class PanoramaModule {
public:
    // tagging_reference is ScriptRuntime::builtin_context_ref (4346D640),
    // shared with CvarModule and MaterialModule and reset for each new state.
    PanoramaModule(PanoramaServices& services, BrowserState& browser, int& tagging_reference)
        : services_(services), browser_(browser), handle_types_ref_(tagging_reference) {}
    PanoramaModule(const PanoramaModule&) = delete;
    PanoramaModule& operator=(const PanoramaModule&) = delete;
    void register_exports(Bridge&); // native binder 0x43346A10
    BrowserState& shutdown_state() noexcept {
        return browser_;
    }
    int open(void*); // 0x4341D995
    int loadstring(void*); // 0x434077F1
    int index(void*); // 0x433E1DD2
    int call(void*); // 0x43321CB2
    int tostring(void*); // 0x433286EB
    int collect_object(void*); // 0x4339E638
    int run_script(void*); // 0x433D19D1
    int collect_script(void*); // 0x43341EC8

private:
    PanoramaServices& services_;
    BrowserState& browser_;
    Bridge* bridge_ = nullptr;
    int& handle_types_ref_; // native shared weak-key registry reference 4346D640
    void* ui_engine() const;
    void* find_panel(const char*) const; // 0x4336B86E
    void* default_panel() const; // 0x4338748D, index zero
    void*** panel_context(void*, void*) const; // 0x433E3C52
    bool unsafe_enabled() const;
    int userdata_tag(void*, int); // 0x433203AB
    void check_userdata(void*, int, int); // 0x433FBA64
    void push_userdata(void*, int); // 0x4334828C, size zero
    int take_handle(void*, V8Local, bool);
    void push_proxy(void*, void*, V8Local, void*, int = -1); // 0x433E03AC
    V8Local to_v8(void*, void*, V8Local, int); // 0x43388ECB / 0x433D6E8B
    void push_v8(void*, void*, V8Local, V8Local, void*, int = -1);
    static int open_entry(void*);
    static int loadstring_entry(void*);
    static int index_entry(void*);
    static int call_entry(void*);
    static int tostring_entry(void*);
    static int collect_object_entry(void*);
    static int run_script_entry(void*);
    static int collect_script_entry(void*);
};

#undef SKEET_V8_METHOD
#undef SKEET_V8_OS
} // namespace skeet::script
