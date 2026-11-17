#pragma once
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <array>
#include <string>
#include "skeet/events/event_name_storage.hpp"

namespace skeet::script {

// The embedded native library exposes the Lua 5.1 C API. Binding an existing
// Lua runtime here keeps allocation, TValue layout and bytecode outside the
// client callback bridge.
struct LuaApi {
    using CFunction = int (*)(void*);
    int (*gettop)(void*);
    void (*settop)(void*, int);
    void (*rawgeti)(void*, int, int);
    int (*type)(void*, int);
    void (*createtable)(void*, int, int);
    void (*pushlstring)(void*, const char*, std::size_t);
    void (*pushinteger)(void*, std::ptrdiff_t);
    void (*settable)(void*, int);
    int (*pcall)(void*, int, int, int);
    const char* (*tolstring)(void*, int, std::size_t*);
    void (*unref)(void*, int, int);
    void (*pushvalue)(void*, int);
    int (*ref)(void*, int);
    void (*pushstring)(void*, const char*);
    void (*gettable)(void*, int);
    std::size_t (*objlen)(void*, int);
    int (*rawequal)(void*, int, int);
    void (*rawseti)(void*, int, int);
    void (*pushnil)(void*);
    void (*checktype)(void*, int, int);
    void (*pushnumber)(void*, double);
    int (*cpcall)(void*, CFunction, void*);
    void (*call)(void*, int, int);
    void (*remove)(void*, int);
    int (*gc)(void*, int, int);
    void (*close)(void*);
    void (*getfield)(void*, int, const char*);
    int (*next)(void*, int);
    void (*rawset)(void*, int);
    int (*toboolean)(void*, int);
    int (*isstring)(void*, int);
    const char* (*checklstring)(void*, int, std::size_t*);
    std::ptrdiff_t (*checkinteger)(void*, int);
    double (*checknumber)(void*, int);
    void (*pushlightuserdata)(void*, void*);
    void* (*touserdata)(void*, int);
    void (*pushcclosure)(void*, CFunction, int);
    void (*setfield)(void*, int, const char*);
    int (*argerror)(void*, int, const char*);
    int (*lerror)(void*, const char*, ...);
    double (*tonumber)(void*, int);
    std::ptrdiff_t (*tointeger)(void*, int);
    void (*pushboolean)(void*, int);
    void* (*newuserdata)(void*, std::size_t);
    int (*getmetatable)(void*, int);
    int (*setmetatable)(void*, int);
    void* (*checkudata)(void*, int, const char*);
    int (*isnumber)(void*, int);
    int (*error)(void*);
    void (*insert)(void*, int);
    void (*lcheckstack)(void*, int, const char*);
    void (*where)(void*, int);
    void (*concat)(void*, int);
};

struct RenderContext {
    const void* queue = nullptr;
    std::int32_t* cursor = nullptr;
};

struct Bridge {
    explicit Bridge(events::EventNameStorage& names)
        : name_storage(names), indicator_text_key(names.alias(events::EventName::text)) {}
    // Game hooks can enter Lua from different OS threads. Recursive entry
    // on the current thread is required by UI, logging, and custom events.
    mutable std::recursive_mutex vm_mutex;
    const LuaApi* api = nullptr;
    void* state = nullptr; // owner + 0x44; 0x433CBCBB
    int indicator_ref = -1; // 0x4346D56C
    int output_ref = -1; // 0x4346D568
    int callback_table_ref = -1; // 0x4346D570
    int runtime_ref = -1; // 0x43468BC0
    const std::uint32_t* frame_number = nullptr; // *0x434678F8
    bool indicator_active = false;
    bool output_active = false;
    RenderContext render_context{}; // 0x4347BD50 / 0x4347BD54
    void* error_context = nullptr;
    void (*report_error)(void*, const char*) = nullptr;
    events::EventNameStorage& name_storage;
    events::EventNameRef name(events::EventName id) const {
        return name_storage.alias(id);
    }
    // The current text pointer is read from its actual process string cell.
    events::EventNameRef indicator_text_key; //0x434793A0
    std::unordered_map<std::uint32_t, bool> callback_active; // 0x4346DA60
    std::unordered_map<std::uint32_t, bool> callback_dirty; // 0x4346DA80
    struct RecentCallbackError {
        std::string message;
        std::uint64_t reported_at = 0;
    };
    std::array<RecentCallbackError, 32> recent_callback_errors{};
    std::array<std::string, 5> aim_miss_reasons{}; //4347930C, initialized by43486D1A
    void finalize_active_callback_storage() noexcept; //4332F1FD, process terminal storage
    void finalize_dirty_callback_storage() noexcept; //4334DA70, independent map
};

bool has_indicator(const Bridge&) noexcept; // 0x4339DF5B
bool has_output(const Bridge&) noexcept; // 0x433E8B30

// Native extension vtable 0x4346FA44: initialization and finalization act on
// the shared callback registry and its two exclusive event references.
struct CallbackState {
    void initialize(Bridge&) const; // 0x4337EF95
    void release(Bridge&) const; // 0x433B3488
};

// Native 0x433F05B5. The renderer passes byte components; the lower-level
// marshaller accepts signed integers without clamping.
void dispatch_indicator(Bridge&, RenderContext, std::string_view, std::int32_t red,
                        std::int32_t green, std::int32_t blue, std::int32_t alpha);
void dispatch_indicator(Bridge&, const void* queue, std::int32_t* cursor, std::string_view text,
                        std::uint32_t abgr);
// 0x433E47AA uses an independent recursion guard and leaves render context alone.
void dispatch_output(Bridge&, std::string_view, std::int32_t red, std::int32_t green,
                     std::int32_t blue, std::int32_t alpha);
void dispatch_output(Bridge&, std::string_view text, std::uint32_t abgr);

// 0x433B1E51 and exclusive branches of 0x433B4C12 / 0x43396EBD.
// Register copies the function at function_index before taking a registry ref.
enum class ExclusiveCallback {
    output,
    indicator
};
void release_reference(const LuaApi&, void* state, int& reference);
void register_exclusive(Bridge&, ExclusiveCallback, int function_index);
void unregister_exclusive(Bridge&, ExclusiveCallback);
// Lua C exports can execute on a coroutine sharing the owner's registry. Its
// function_index is relative to that invoking state, rather than bridge.state.
void register_exclusive(Bridge&, void* invoking_state, ExclusiveCallback, int function_index);
void unregister_exclusive(Bridge&, void* invoking_state, ExclusiveCallback);

// 0x433A16CC leaves registry table and named callback list on the Lua stack
// when found. On absence it restores the previous top.
bool push_callback_list(const Bridge&, void* state, const char* name);

// Lua array helpers preserve relative stack indices exactly as in the client.
int find_callback(const LuaApi&, void* state, int table_index, int value_index); // 0x433FAA98
void remove_callback(const LuaApi&, void* state, int table_index, int index); // 0x433843FB
void compact_callback_list(const LuaApi&, void* state, int table_index); // 0x433B8905

using Marshaller = int (*)(Bridge&, void*);
// 0x433BA445: one marshalling pass, copied arguments for every live callback.
// A marshaller result of -1 cancels callback invocation; errors do not stop
// subsequent callbacks. The return value counts successful protected calls.
int dispatch_callbacks(Bridge&, const char* name, Marshaller, void* context);
void dispatch_custom_event(Bridge&, void* state, const char* name, bool has_argument);
// 433A01A8 reads its shared argument table before popping it and before
// compacting callbacks/restoring the active flag. The consumer implements
// that concrete event's result conversion while its arguments remain live.
using ArgumentConsumer = void (*)(Bridge&, void* context, int completed);
int dispatch_callbacks(Bridge&, const char* name, Marshaller, void* context, ArgumentConsumer);

// 0x4334EAB1 builds one table with a signed integer under the separately
// initialized 14-byte field name (0x43479580). 0x433F48F6 supplies the value.
int dispatch_integer_event(Bridge&, const char* event_name, const char* field_name,
                           std::int32_t value);
// The predefined command-number field remains live until its pushlstring,
// after creation of the event table. Dynamic fields use the overload above.
int dispatch_integer_event(Bridge&, const char* event_name, events::EventNameRef field_name,
                           std::int32_t value);

struct AimShot {
    std::int32_t id; // native shot + 0x14
    std::int32_t target; // native shot + 0x94
    std::int32_t hitgroup; // native shot + 0xC0
    float hit_chance; // native shot + 0xC8, fraction
    std::int32_t tick; // native shot + 0x144
};

// 0x43342FA6 -> 0x433EAFA1. The wrapper passes a local reason copy; the
// marshaller clamps that copy to zero when it exceeds four.
int dispatch_aim_miss(Bridge&, const AimShot&, std::uint32_t reason);
// 0x43406164 -> 0x433CFB6A; actual target, hitgroup and damage are independent
// of the predicted fields stored in AimShot.
int dispatch_aim_hit(Bridge&, const AimShot&, std::int32_t target, std::int32_t hitgroup,
                     std::int32_t damage);

// 0x433A172C -> lua_cpcall(0x4340183B): calls runtime registry table slot 2.
void notify_shutdown(Bridge&);
struct ShutdownState;
void destroy_runtime(ShutdownState&);

// 0x4334DEFF: ASCII-insensitive FNV-1a followed by the native xor mask.
std::uint32_t callback_hash(const char* name) noexcept;

} // namespace skeet::script
