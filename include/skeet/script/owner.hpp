#pragma once
#include "skeet/script/modules.hpp"
#include <array>

struct _EXCEPTION_POINTERS;
namespace skeet::client {
struct ExitCallbacks;
}

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_SCRIPT_OWNER_NT __stdcall
#define SKEET_SCRIPT_OWNER_MEMBER __thiscall
#else
#define SKEET_SCRIPT_OWNER_NT
#define SKEET_SCRIPT_OWNER_MEMBER
#endif

namespace skeet::script {
class ScriptOwner;

// Native RTL_BITMAP4346D638. The initialized buffer belongs to the actual
// process bootstrap and remains alive through removal of the Lua handler.
struct ScriptExecutableBitmap {
    std::uint32_t bit_count;
    std::uint32_t* buffer;
};
struct ScriptOwnerSystem {
    using ExceptionHandler = long(SKEET_SCRIPT_OWNER_NT*)(::_EXCEPTION_POINTERS*);
    void*(SKEET_SCRIPT_OWNER_NT* add_handler)(unsigned long, ExceptionHandler);
    unsigned long(SKEET_SCRIPT_OWNER_NT* remove_handler)(void*);
    void(SKEET_SCRIPT_OWNER_NT* set_bit)(ScriptExecutableBitmap*, unsigned long);
};
ScriptOwnerSystem native_script_owner_system();

struct ScriptExecutableRange {
    std::uintptr_t image_base; //43467E70
    std::uint32_t image_size; //434679DC
    ScriptExecutableBitmap& bitmap;
};
struct ScriptOwnerCodeState {
    std::uint32_t maximum_allocations = 0; //4347CA20
    std::uintptr_t free_begin = 0; //4347BD3C
    std::uintptr_t allocation_cursor = 0; //4347BD58
};
struct ScriptOwnerBindings {
    ScriptOwner*& current_owner; //43475B00
    engine::OwnerRegistryBinding primary; //43475B78/7C/80
    ScriptExecutableRange executable;
    ScriptOwnerSystem system;
    skeet::client::ExitCallbacks& exit_callbacks;
};

// 43387448 allocation and4347FF57 construction are separate operations in
// this API. The caller retains the nullable allocation result; construction
// requires a successful allocation and does not invent its caller's failure
// branch. Destruction through primary slot0 releases this exact allocation.
struct ScriptOwnerStorage {
    void* address;
    engine::JobMemory memory;
};
ScriptOwnerStorage allocate_script_owner_storage(engine::JobMemory);
ScriptOwner& construct_script_owner(ScriptOwnerStorage, ScriptRuntime&, ScriptOwnerBindings,
                                    ScriptModulesBindings);
void destroy_script_owner(ScriptOwner*, ScriptOwner*& current_owner); //43387A8F also accepts null

class ScriptOwner final {
public:
    ScriptOwner(const ScriptOwner&) = delete;
    ScriptOwner& operator=(const ScriptOwner&) = delete;
    ScriptModulesOwner& modules() noexcept {
        return modules_;
    }
    ScriptOwnerCodeState& code_state() noexcept {
        return code_state_;
    }
    void initialize_process(); //4347EA9D, primary slot1
    void remove_exception_handler(); //4332002B, slot4
    void update_extensions(); //4332A8E1, slot6
    void level_init(); //4332A674, slot7
    void destroy(); //43387A8F, slot0
    void bind_shutdown(skeet::client::ShutdownRuntime&);

private:
    using PrimaryEntry = void(SKEET_SCRIPT_OWNER_MEMBER*)(void*);
    struct PrimaryEndpoint {
        const PrimaryEntry* vtable = nullptr;
        ScriptOwner* owner;
        PrimaryEndpoint(ScriptOwner&, engine::OwnerRegistryBinding);
    } primary_;
    ScriptRuntime& runtime_;
    ScriptOwner*& current_owner_;
    engine::JobMemory memory_;
    ScriptExecutableRange executable_;
    ScriptOwnerSystem system_;
    skeet::client::ExitCallbacks& exit_callbacks_;
    ScriptOwnerCodeState code_state_;
    void* exception_handler_ = nullptr; //native owner+284
    ScriptModulesOwner modules_;

    ScriptOwner(ScriptOwnerStorage, ScriptRuntime&, ScriptOwnerBindings, ScriptModulesBindings);
    ~ScriptOwner() noexcept(false);
    void initialize_extension(RegisteredExtension);
    void initialize_callback_process(); //43486D1A
    static const std::array<PrimaryEntry, 9> primary_table_;
    static void SKEET_SCRIPT_OWNER_MEMBER destroy_entry(void*);
    static void SKEET_SCRIPT_OWNER_MEMBER initialize_entry(void*);
    static void SKEET_SCRIPT_OWNER_MEMBER empty_entry(void*); //433CCE8E
    static void SKEET_SCRIPT_OWNER_MEMBER remove_entry(void*);
    static void SKEET_SCRIPT_OWNER_MEMBER update_entry(void*);
    static void SKEET_SCRIPT_OWNER_MEMBER level_entry(void*);
    static long SKEET_SCRIPT_OWNER_NT exception_handler(::_EXCEPTION_POINTERS*);
    friend ScriptOwner& construct_script_owner(ScriptOwnerStorage, ScriptRuntime&,
                                               ScriptOwnerBindings, ScriptModulesBindings);
    friend void destroy_script_owner(ScriptOwner*, ScriptOwner*&);
};
} // namespace skeet::script

#undef SKEET_SCRIPT_OWNER_NT
#undef SKEET_SCRIPT_OWNER_MEMBER
