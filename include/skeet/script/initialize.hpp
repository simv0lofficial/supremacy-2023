#pragma once
#include "bridge.hpp"
#include <span>
#include <string>
#include <vector>
#include <array>
#include <unordered_set>

namespace skeet::script {
class UiBindings;
class UiModule;
class JsonModule;
class VectorModule;
class CompatModule;
class ExtensionRegistry;
struct BuiltinModule {
    std::uint64_t modified = 0;
    bool enabled = false;
    std::string name;
    std::vector<std::byte> source;
    bool encrypted = false;
};
struct RequiredModule {
    std::string name;
    std::vector<std::byte> source;
    bool encrypted = false;
};
struct ProcessTimes {
    std::uint64_t created = 0, exited = 0, kernel = 0, user = 0;
};
struct BuiltinCipher {
    using Query = std::int32_t(__stdcall*)(void*, std::uint32_t, void*, std::uint32_t,
                                           std::uint32_t*);
    std::uint32_t process_id = 0, peb = 0;
    Query query = nullptr;
    std::array<std::uint8_t, 16> key() const; // 434198F1
    void transform(std::span<std::byte> bytes) const; // 4331E6F0 / 4334E073
    static BuiltinCipher system();
};
struct LuaVmLayout {
    std::uint32_t global_reference_offset = 8;
    std::uint32_t global_reference_size = 4;
    std::uint32_t machine_code_limit_offset = 0x7c0;
};

// These entries are the Lua runtime boundary; library initialization and client
// module registration are concrete code in ScriptRuntime.
struct InitializationApi {
    void* (*newstate)() = nullptr; // luaL_newstate, 43433899
    int (*setmode)(void*, int, int) = nullptr; // luaJIT_setmode, 43432B61
    LuaApi::CFunction (*atpanic)(void*, LuaApi::CFunction) = nullptr; // 434256EA
    void (*openlibs)(void*) = nullptr; // luaL_openlibs, 43437EB6
    LuaVmLayout layout;
};

class ScriptRuntime {
public:
    ScriptRuntime(Bridge& bridge, InitializationApi api, bool& actual_scripting_enabled) noexcept
        : bridge(bridge), api(api), enabled(actual_scripting_enabled) {}
    ~ScriptRuntime();
    ScriptRuntime(const ScriptRuntime&) = delete;
    ScriptRuntime& operator=(const ScriptRuntime&) = delete;
    Bridge& bridge;
    InitializationApi api;
    UiBindings* ui_bindings = nullptr;
    UiModule* ui_module = nullptr;
    JsonModule* json_module = nullptr;
    VectorModule* vector_module = nullptr;
    CompatModule* compat_module = nullptr;
    ExtensionRegistry* extensions = nullptr;
    std::span<BuiltinModule> builtins;
    std::span<RequiredModule> required_modules; // 434792D0..D4, supremacy/ and gamesense/ loader
    bool& enabled; // 434688BD, canonical startup availability; distinct from menu script settings
    int mode = 0x100; // requested43475A80
    int applied_mode = 0x100; // 43468580
    bool initializing = false; // 4347582C
    bool loading_builtin = false; // 4347CA10
    std::uint32_t executable_block_count = 0; // 4347CA24, 64KB reserved JIT arenas
    int builtin_context_ref = -1; // 4346D640, shared weak-key cvar/material/panorama userdata tags
    BuiltinCipher cipher;
    std::unordered_set<std::uintptr_t> unavailable_ffi_addresses;

    void* ensure_state(); // 433C05F9
    void* initialize() {
        return ensure_state();
    }
    void register_modules(); // 433EC568, called through lua_cpcall
    void encrypt_builtin(BuiltinModule&);
    void encrypt_required(RequiredModule&);
    int run_builtin(BuiltinModule&); // 4336EF9E, source must be plaintext
    void run_builtins(); // 433824BE
    void install_loaders(); // 433B6C6E
private:
    static int bootstrap(void*);
    static int panic(void*);
    static int loader(void*);
};
using RuntimeBootstrap = ScriptRuntime;
void release_runtime_owner(void* state) noexcept;
} // namespace skeet::script
