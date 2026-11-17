#pragma once
#include "skeet/script/material_api.hpp"
#include <span>

namespace skeet::script {

struct MaterialMethodRegistration {
    const char* name;
    LuaApi::CFunction function;
    std::uint32_t native_address;
};

// The material userdata's __index registry constructed by 0x43486066 and
// installed through native global 0x4347579C. These are separate from the
// materialsystem module's eight public functions.
class MaterialMethods {
public:
    static std::span<const MaterialMethodRegistration> registrations() noexcept;
    static int get_name_entry(void*); // 0x43408733
    static int reload_entry(void*); // 0x433D2DE4
    static int alpha_modulate_entry(void*); // 0x433D195E
    static int color_modulate_entry(void*); // 0x433982F3
    static int set_shader_param_entry(void*); // 0x4337F6D1
    static int get_shader_param_entry(void*); // 0x4339CA67
    static int set_material_var_flag_entry(void*); // 0x43328673
    static int get_material_var_flag_entry(void*); // 0x433E9AEF
private:
    static MaterialModule::Object* object(MaterialModule&, void*, int required_arguments);
    static bool parameter(MaterialModule&, void*, std::int32_t&, const char*&);
};

} // namespace skeet::script
