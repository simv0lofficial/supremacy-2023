#pragma once
#include "skeet/script/shutdown.hpp"
#include <cstdint>

namespace skeet::effects {
class ModelRenderRuntime;
}

namespace skeet::script {
class MaterialMethodRegistry;

#if defined(_M_IX86) || defined(__i386__)
using MaterialVariableFactory = void*(__fastcall*)(void*, const char*);
#else
using MaterialVariableFactory = void* (*)(void*, const char*);
#endif
struct MaterialInterfaces {
    void* engine_client = nullptr; // 0x4346A514, IsInGame slot 26
    void* material_system = nullptr; // 0x4346A4E0
    void* entity_list = nullptr; // 0x4346A890
    void* model_info = nullptr; // 0x4346A614
    void* studio_data_cache = nullptr; // 0x43468A4C
    const std::int32_t* activity = nullptr; // 0x4347C9FC, 1 rejects engine access
    const bool* unloading = nullptr; // 0x43475895
    MaterialVariableFactory create_variable = nullptr; // 43467A68, internal materialsystem fastcall
};

// 0x433E2E5E. Lua owns the cache, userdata and their metatable. This owner and
// its engine interfaces must outlive the bound Lua state.
class MaterialModule {
public:
    MaterialModule(MaterialInterfaces& interfaces, effects::ModelRenderRuntime* models,
                   MaterialRefs& references, int& tagging_reference,
                   MaterialMethodRegistry& method_registry)
        : interfaces_(interfaces), models_(models), references_(references),
          handle_types_ref_(tagging_reference), method_registry_(method_registry) {}
    MaterialModule(const MaterialModule&) = delete;
    MaterialModule& operator=(const MaterialModule&) = delete;
    MaterialRefs& shutdown_references() noexcept {
        return references_;
    }
    void register_exports(Bridge&);

    int find_material(void*); // 0x433D2737
    int find_materials(void*); // 0x4333D05F
    int find_texture(void*); // 0x4332900B
    int get_model_materials(void*); // 0x434070E8
    int override_material(void*); // 0x4335852D
    int arms_material(void*); // 0x4337D885
    int chams_material(void*); // 0x433AB527
    int viewmodel_material(void*); // 0x433981EE

private:
    friend class MaterialMethods;
    friend class MaterialValues;
    struct Object {
        void* material;
        bool loaded, changed;
    };
    MaterialInterfaces& interfaces_;
    effects::ModelRenderRuntime* models_;
    MaterialRefs& references_;
    Bridge* bridge_ = nullptr;
    int& handle_types_ref_; // shared 0x4346D640; reset by 0x433C067A
    MaterialMethodRegistry& method_registry_;
    static void* (*context_)(void*, int);
    static MaterialModule& owner(void*, int upvalue = -10005);
    bool ready() const;
    void push_handle_types(void*); // 0x433880A2 / 0x4337FE16
    void tag_top(void*, std::uint32_t);
    void* tagged(void*, int, std::uint32_t); // 0x4338A612
    void push_object(void*, void*, const char*); // 0x433CB5B4
    void push_material(void*, const char*, bool); // 0x433CD6DC
    void push_named_material(void*, void*); // 0x43341199
    void* search(const char*, bool); // 0x4339FD67
    bool load(Object&); // 0x434035AC
    int cached_material(void*, std::size_t, std::size_t, const char*);
    int collect(void*); // 0x433E9C60
    static int find_material_entry(void*);
    static int find_materials_entry(void*);
    static int find_texture_entry(void*);
    static int get_model_materials_entry(void*);
    static int override_material_entry(void*);
    static int arms_material_entry(void*);
    static int chams_material_entry(void*);
    static int viewmodel_material_entry(void*);
    static int collect_entry(void*);
};

} // namespace skeet::script
