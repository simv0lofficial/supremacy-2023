#pragma once
#include "skeet/script/bridge.hpp"
#include <array>
#include <cstdint>

namespace skeet::script {

// The captured ConVar layout is shared by effects/world and misc/fov.
void* cvar_parent(void* variable);
float cvar_float(void* variable);
std::int32_t cvar_int(void* variable);
void set_cvar_raw_float(void* variable, float value);
void set_cvar_raw_int(void* variable, std::int32_t value);
void* find_cvar(void* engine_cvar, const char* name); // ICvar::FindCommandBase, slot 15

// CCommand supplied to both function and interface command callbacks. The
// pointer array naturally follows the ABI of the rebuilt engine boundary.
struct CvarCommandArguments {
    std::int32_t count = 0;
    std::int32_t command_name_length = 0;
    std::array<char, 512> command_line{};
    std::array<char, 512> argument_buffer{};
    std::array<const char*, 64> arguments{};
    std::int32_t unknown = -1;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(CvarCommandArguments) == 0x50C);
#endif

// Native binder 433ECCAB. The shared tagging registry is native 4346D640;
// unrestricted is the feature byte at 434758D4. Both references outlive Lua.
class CvarModule {
public:
    CvarModule(void* engine_cvar, int& tagging_ref, const std::uint8_t& unrestricted) noexcept
        : engine_cvar_(engine_cvar), tagging_ref_(tagging_ref), unrestricted_(unrestricted) {}
    CvarModule(const CvarModule&) = delete;
    CvarModule& operator=(const CvarModule&) = delete;
    void register_exports(Bridge&);
    int index(void*); // 4332A4B1
    int set_string(void*); // 433598FF
    int get_string(void*); // 433E4364
    int set_float(void*); // 4335C1BE
    int set_raw_float(void*); // 433F4A95
    int get_float(void*); // 43330971
    int set_int(void*); // 433CFD5D
    int set_raw_int(void*); // 43343295
    int get_int(void*); // 433EEDAD
    int invoke_callback(void*); // 4341B486

private:
    void* engine_cvar_;
    int& tagging_ref_;
    const std::uint8_t& unrestricted_;
    Bridge* bridge_ = nullptr;
    void push_tagging_table(void*); // 433880A2 / 4337FE16
    void push_tagged_pointer(void*, void*); // 433A05EB
    void* argument(void*, int minimum); // 4340BB1F / 4338A612
    void invoke_variable(void*, void*); // 433EB1BC
    void invoke_command(void*, void*); // 433B0620 / 433D2A0D / 433CB651
    static int index_entry(void*);
    static int set_string_entry(void*);
    static int get_string_entry(void*);
    static int set_float_entry(void*);
    static int set_raw_float_entry(void*);
    static int get_float_entry(void*);
    static int set_int_entry(void*);
    static int set_raw_int_entry(void*);
    static int get_int_entry(void*);
    static int invoke_callback_entry(void*);
};

} // namespace skeet::script
