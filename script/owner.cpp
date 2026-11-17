#include "skeet/script/owner.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/engine/unload.hpp"
#include "detail.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

#if defined(__i386__) && defined(__clang__)
extern "C" int skeet_lua_has_native_unwinder(const void* limit);
#endif

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_SCRIPT_PRIMARY __thiscall
#define SKEET_SCRIPT_EXCEPTION __stdcall
#define SKEET_SCRIPT_FAST __fastcall
#else
#define SKEET_SCRIPT_PRIMARY
#define SKEET_SCRIPT_EXCEPTION
#define SKEET_SCRIPT_FAST
#endif

namespace skeet::script {
namespace {
MaterialMethodRegistry* process_materials = nullptr; //the process4347579C owner

void release_process_materials() { //4334ED1D
    process_materials->destroy_process();
}

void* allocate_aligned(engine::JobMemory memory, std::size_t size) { //43387448
    if (!size)
        return nullptr;
    auto* allocation = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!allocation)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - address);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}

void free_aligned(engine::JobMemory memory, void* object) { //433837D0
    if (!object)
        return;
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}

int marshal_no_arguments(Bridge&, void*) {
    return 0;
} //43365AE1 ->43336942

#if defined(__i386__) && defined(__clang__)
// 4337EB9B accepts the saved checkpoint in ECX and the exception result in
// EDX. The installed VEH redirects CONTEXT to this exact restoration path.
[[noreturn]] __attribute__((naked, noinline, fastcall)) void
resume_exception_checkpoint(const detail::X86Checkpoint*, std::uint32_t) {
    __asm__ __volatile__("movl %edx, %eax\n\t"
                         "movl 0(%ecx), %edi\n\t"
                         "movl 4(%ecx), %esi\n\t"
                         "movl 8(%ecx), %ebp\n\t"
                         "movl 16(%ecx), %ebx\n\t"
                         "movl 24(%ecx), %edx\n\t"
                         "movl %edx, %fs:0\n\t"
                         "movl 12(%ecx), %esp\n\t"
                         "jmpl *20(%ecx)");
}
#else
[[noreturn]] void SKEET_SCRIPT_FAST resume_exception_frame(detail::ActiveFrame* active,
                                                           std::uint32_t code) {
    auto* frame = reinterpret_cast<detail::CallbackFrame*>(reinterpret_cast<std::byte*>(active) -
                                                           offsetof(detail::CallbackFrame, active));
    std::longjmp(frame->checkpoint, static_cast<int>(code));
}
#endif
}

ScriptOwnerSystem native_script_owner_system() {
    const auto native = GetModuleHandleA("ntdll.dll");
    const auto add = native ? GetProcAddress(native, "RtlAddVectoredExceptionHandler") : nullptr;
    const auto remove =
        native ? GetProcAddress(native, "RtlRemoveVectoredExceptionHandler") : nullptr;
    const auto bit = native ? GetProcAddress(native, "RtlSetBit") : nullptr;
    if (!add || !remove || !bit)
        throw std::runtime_error("Required native script primary imports are missing");
    ScriptOwnerSystem result;
    result.add_handler = reinterpret_cast<decltype(result.add_handler)>(add);
    result.remove_handler = reinterpret_cast<decltype(result.remove_handler)>(remove);
    result.set_bit = reinterpret_cast<decltype(result.set_bit)>(bit);
    return result;
}

const std::array<ScriptOwner::PrimaryEntry, 9> ScriptOwner::primary_table_{
    destroy_entry, initialize_entry, empty_entry, empty_entry, remove_entry,
    empty_entry,   update_entry,     level_entry, empty_entry}; //434702B4

ScriptOwner::PrimaryEndpoint::PrimaryEndpoint(ScriptOwner& object,
                                              engine::OwnerRegistryBinding binding)
    : owner(&object) {
    binding.append(this); //433676A0 occurs before every secondary registration
}

ScriptOwnerStorage allocate_script_owner_storage(engine::JobMemory memory) {
    if (!memory.allocate || !memory.free)
        throw std::invalid_argument("Script owner requires its process heap imports");
    return {allocate_aligned(memory, sizeof(ScriptOwner)), memory};
}

ScriptOwner& construct_script_owner(ScriptOwnerStorage storage, ScriptRuntime& runtime,
                                    ScriptOwnerBindings bindings, ScriptModulesBindings modules) {
    if (!storage.address)
        throw std::invalid_argument(
            "Script owner construction requires successful native allocation");
    if (!bindings.system.add_handler || !bindings.system.remove_handler ||
        !bindings.system.set_bit || !bindings.executable.bitmap.buffer)
        throw std::invalid_argument(
            "Script primary requires its native handler imports and initialized bitmap");
    if (&bindings.primary.registry != &modules.unload_state.owners ||
        bindings.primary.memory.process_heap != storage.memory.process_heap)
        throw std::invalid_argument(
            "Script primary construction requires the canonical owner registry and process heap");
    ScriptModulesOwner::validate_construction(runtime, modules);
    auto* owner =
        ::new (storage.address) ScriptOwner(storage, runtime, bindings, std::move(modules));
    bindings.current_owner = owner;
    return *owner;
}

ScriptOwner::ScriptOwner(ScriptOwnerStorage storage, ScriptRuntime& runtime,
                         ScriptOwnerBindings bindings, ScriptModulesBindings modules)
    : primary_(*this, bindings.primary), runtime_(runtime), current_owner_(bindings.current_owner),
      memory_(storage.memory), executable_(bindings.executable), system_(bindings.system),
      exit_callbacks_(bindings.exit_callbacks), modules_(runtime, std::move(modules)) {
    primary_.vtable = primary_table_.data(); //4347FFE2
    modules_.publish_command_hud_entity_tables(modules.command_callbacks, modules.entity_callbacks);
}

ScriptOwner::~ScriptOwner() noexcept(false) = default;

void ScriptOwner::initialize_callback_process() { //43486D1A
    auto* replacement = static_cast<client::ScriptCommand*>(allocate_aligned(memory_, 0x34));
    if (replacement)
        std::memset(replacement, 0, 0x34);
    auto& current = modules_.command_snapshot();
    if (replacement != current) {
        free_aligned(memory_, current);
        current = replacement;
    }
    modules_.bridge().aim_miss_reasons = {"?", "spread", "prediction error", "death",
                                          "unregistered shot"};
}

void ScriptOwner::initialize_extension(RegisteredExtension extension) {
    switch (extension) {
    case RegisteredExtension::flags: //4346F3F8 slot0=4347E307
        initialize_client_clock(modules_.client().bindings.clock);
        break;
    case RegisteredExtension::no_op_434792f8: //4346F618 slot0=43482538
        modules_.database().initialize_process();
        break;
    case RegisteredExtension::callbacks: //4346FA44 slot0=43486D1A
        initialize_callback_process();
        break;
    case RegisteredExtension::materials: //4346FE7C slot0=43486066
        process_materials = &modules_.material_registry();
        process_materials->initialize_process();
        if (process_materials->available())
            (void)exit_callbacks_.append(release_process_materials);
        break;
    case RegisteredExtension::no_op_434792f0: //4347D93C is an actual ret
    case RegisteredExtension::no_op_434792f4:
    case RegisteredExtension::no_op_434792fc:
    case RegisteredExtension::no_op_4347935c:
    case RegisteredExtension::files:
    case RegisteredExtension::browser:
    case RegisteredExtension::no_op_4347965c:
    case RegisteredExtension::ui: //the remaining slot0 entries are433CCE8E
        break;
    }
}

void ScriptOwner::initialize_process() { //4347EA9D
    exception_handler_ = system_.add_handler(1, &exception_handler);
    code_state_.maximum_allocations = 0x10;
    code_state_.free_begin =
        (executable_.image_base + executable_.image_size - 0x10000) & ~std::uintptr_t(0xfff);
    code_state_.allocation_cursor = executable_.image_base;
    runtime_.executable_block_count =
        static_cast<std::uint32_t>((code_state_.free_begin - executable_.image_base) >> 16);
    const auto end = code_state_.free_begin + 0x10000;
    for (auto page = executable_.image_base; page < end; page += 0x20000)
        system_.set_bit(&executable_.bitmap, static_cast<unsigned long>(page >> 17));
    const auto order = modules_.extensions().registered_order();
    for (const auto extension : order)
        initialize_extension(extension);
}

void ScriptOwner::remove_exception_handler() { //4332002B
    if (exception_handler_) {
        (void)system_.remove_handler(exception_handler_);
        exception_handler_ = nullptr;
    }
}

void ScriptOwner::update_extensions() { //4332A8E1
    if (!modules_.bridge().state)
        return;
    // All twelve recovered extension tables have the exact433C95B7 ret4 in
    // slot2. The native traversal therefore has no further state effects.
}

void ScriptOwner::level_init() { //4332A674; key43479540 initializer43481405..83
    auto& bridge = modules_.bridge();
    if (bridge.state)
        dispatch_callbacks(bridge, bridge.name(events::EventName::level_init).get(),
                           marshal_no_arguments, nullptr);
}

void ScriptOwner::destroy() { //43387A8F
    destroy_script_owner(this, current_owner_);
}

void destroy_script_owner(ScriptOwner* owner, ScriptOwner*& current_owner) {
    current_owner = nullptr;
    if (!owner)
        return;
    const auto memory = owner->memory_;
    owner->~ScriptOwner();
    free_aligned(memory, owner);
}

void ScriptOwner::bind_shutdown(client::ShutdownRuntime& finalizer) {
    if (&finalizer.exit_callbacks != &exit_callbacks_)
        throw std::invalid_argument(
            "Script process initialization and finalization require the same exit callback state");
    modules_.bind_shutdown(finalizer);
}

void SKEET_SCRIPT_PRIMARY ScriptOwner::destroy_entry(void* receiver) {
    static_cast<PrimaryEndpoint*>(receiver)->owner->destroy();
}
void SKEET_SCRIPT_PRIMARY ScriptOwner::initialize_entry(void* receiver) {
    static_cast<PrimaryEndpoint*>(receiver)->owner->initialize_process();
}
void SKEET_SCRIPT_PRIMARY ScriptOwner::empty_entry(void*) {} //433CCE8E
void SKEET_SCRIPT_PRIMARY ScriptOwner::remove_entry(void* receiver) {
    static_cast<PrimaryEndpoint*>(receiver)->owner->remove_exception_handler();
}
void SKEET_SCRIPT_PRIMARY ScriptOwner::update_entry(void* receiver) {
    static_cast<PrimaryEndpoint*>(receiver)->owner->update_extensions();
}
void SKEET_SCRIPT_PRIMARY ScriptOwner::level_entry(void* receiver) {
    static_cast<PrimaryEndpoint*>(receiver)->owner->level_init();
}

long SKEET_SCRIPT_EXCEPTION
ScriptOwner::exception_handler(::_EXCEPTION_POINTERS* exception) { //433592DE
    // The source-linked LuaJIT uses Windows SEH for ordinary Lua errors.
    // Its protected calls must receive these before the outer native recovery
    // frame; intercepting them here skips pcall/xpcall and their error values.
    if ((exception->ExceptionRecord->ExceptionCode & 0xffffff00u) == 0xe24c4a00u)
        return EXCEPTION_CONTINUE_SEARCH;
    for (auto* frame = detail::RtlGetFrame(); frame; frame = frame->previous) {
        auto hash = 0x811c9dc5u;
        for (unsigned shift = 0; shift != 32; shift += 8)
            hash = (hash ^ ((frame->flags >> shift) & 255u)) * 0x01000193u;
        if ((hash ^ 0x92ca5df1u) != 0xe18c2fa4u)
            continue;
        auto* callback = reinterpret_cast<detail::CallbackFrame*>(
            reinterpret_cast<std::byte*>(frame) - offsetof(detail::CallbackFrame, active));
#if defined(__i386__) && defined(__clang__)
        const auto code = exception->ExceptionRecord->ExceptionCode;
        if ((code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_IN_PAGE_ERROR) &&
            skeet_lua_has_native_unwinder(&callback->active))
            return EXCEPTION_CONTINUE_SEARCH;
#endif
        callback->exception_code = exception->ExceptionRecord->ExceptionCode;
        callback->exception_address =
            reinterpret_cast<std::uintptr_t>(exception->ExceptionRecord->ExceptionAddress);
        auto* context = exception->ContextRecord;
#if defined(__i386__) && defined(__clang__)
        context->Ecx = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(frame) - 0x1c);
        context->Edx = exception->ExceptionRecord->ExceptionCode;
        context->Eip =
            static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&resume_exception_checkpoint));
#elif defined(_M_IX86) || defined(__i386__)
        context->Ecx = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(frame));
        context->Edx = exception->ExceptionRecord->ExceptionCode;
        context->Eip =
            static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&resume_exception_frame));
#else
        context->Rcx = reinterpret_cast<DWORD64>(frame);
        context->Rdx = exception->ExceptionRecord->ExceptionCode;
        context->Rip = reinterpret_cast<DWORD64>(&resume_exception_frame);
#endif
        return -1;
    }
    return 0;
}
} // namespace skeet::script

#undef SKEET_SCRIPT_PRIMARY
#undef SKEET_SCRIPT_EXCEPTION
#undef SKEET_SCRIPT_FAST
