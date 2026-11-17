// Input Subsystem Monolith

#include "util/address.h"
using skeet::field;
using skeet::store;
using skeet::add;
using skeet::subtract;

#include "skeet/input/runtime.hpp"
#include <bit>
#include <stdexcept>
#include "skeet/input/native_window_lifecycle.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/config_storage/settings_persistence.hpp"
#include "skeet/network/update_observers.hpp"
#include <cstring>
#include <new>
#include <type_traits>
#include <windows.h>
#include <intrin.h>
#include "skeet/input/window_hook.hpp"
#include "skeet/engine/common_controls_initialization.hpp"


// Subcomponent: configuration.cpp

namespace skeet::input {
void initialize_menu_hotkey(ConfigRegistries& configuration,
                            RegisteredHotkey& menu_key) noexcept { // 434928A2
    initialize_hotkey(configuration.hotkeys, menu_key, 0x2e, HotkeyMode::hold);
}
}

// Subcomponent: cursor.cpp

namespace skeet::input {
void Runtime::enable_cursor() { // 0x433C0428
    if (disabled)
        return;
    if (!cursor_context)
        cursor_context = engine::invoke<std::uint32_t>(interfaces.cursor_contexts, 9);
    if (cursor_context)
        engine::invoke<void>(interfaces.cursor_contexts, 11, cursor_context, true);
}
void Runtime::release_cursor() { // 0x4339F7FB
    Point position{};
    if (cursor_context) {
        if (interfaces.in_game()) {
            engine::invoke<void>(interfaces.engine_client, 5, &position.x, &position.y);
            position.x >>= 1;
            position.y >>= 1;
            if (engine::invoke<bool>(interfaces.cursor_contexts, 16, cursor_context))
                engine::invoke<void>(interfaces.cursor_contexts, 15, cursor_context, position.x,
                                     position.y);
        }
        engine::invoke<void>(interfaces.cursor_contexts, 11, cursor_context, false);
    }
    if (interfaces.in_game())
        engine::invoke<void>(interfaces.input_system, 61, &position.x, &position.y);
}
void Runtime::destroy_cursor() { // 0x433675D9
    if (!interfaces.cursor_contexts || !cursor_context)
        return;
    engine::invoke<void>(interfaces.cursor_contexts, 11, cursor_context, true);
    if (engine::invoke<bool>(interfaces.cursor_contexts, 16, cursor_context))
        engine::invoke<void>(interfaces.cursor_contexts, 10);
    else
        engine::invoke<void>(interfaces.cursor_contexts, 11, cursor_context, false);
    cursor_context = 0;
}
void Runtime::toggle_menu() { // 0x4338579D
    if (main_menu->input.mouse_flags & 1)
        release_cursor();
    else
        enable_cursor();
    if (!(main_menu->input.mouse_flags & 1)) {
        main_menu->open();
        return;
    }
    if (main_menu->transition_state == 1)
        return;
    for (auto* window = windows; window; window = window->next) {
        if (!(window->input.mouse_flags & 1))
            continue;
        window->transition_started = window->read_ticks(window->clock_context);
        window->transition_state = 1;
    }
}
}

// Subcomponent: dispatch.cpp

namespace skeet::input {
bool Runtime::dispatch(ui::WindowMessage& original) { // 0x43405C69
    const auto previous_mouse_buttons = mouse_buttons;
    if (disabled)
        return false;
    auto message = original;
    auto& input = message.input;
    switch (input.message) {
    case 0x203:
        input.message = 0x201;
        break;
    case 0x206:
        input.message = 0x204;
        break;
    case 0x209:
        input.message = 0x207;
        break;
    case 0x20d:
        input.message = 0x20b;
        break;
    case 8:
        if (keys[0] & (1u << 18)) {
            input.message = 0x101;
            input.parameter = 18;
            input.key_data = 0x8000;
        }
        break;
    case 0x104:
        input.message = 0x100;
        break;
    case 0x105:
        input.message = 0x101;
        break;
    }
    if (input.message == 0x100) {
        if (!menu_hotkey)
            throw std::logic_error("Input menu hotkey is not bound to its registered setting");
        if (input.parameter == menu_hotkey->binding.packed >> 2)
            toggle_menu();
    }
    if (input.message - 0x200 > 14 &&
        (input.message < 0x100 || (input.message > 0x102 && input.message != 0xa090)))
        return false;
    const auto previous_keys = keys;
    bool consumed = false;
    for (auto* window = windows; window; window = window->next) {
        if (window->dispatch(message)) {
            consumed = true;
            break;
        }
    }
    update_keys(input, consumed);
    const auto newly_pressed_mouse =
        static_cast<std::uint16_t>((mouse_buttons ^ previous_mouse_buttons) & mouse_buttons);
    if (consumed) {
        captured_mouse_buttons = static_cast<std::uint16_t>(
            (newly_pressed_mouse | captured_mouse_buttons) & mouse_buttons);
        return true;
    }
    captured_mouse_buttons =
        static_cast<std::uint16_t>(~newly_pressed_mouse & captured_mouse_buttons & mouse_buttons);
    if (captured_mouse_buttons &&
        (input.message == 0x200 || input.message == 0x201 || input.message == 0x202 ||
         input.message == 0x204 || input.message == 0x205 || input.message == 0x207 ||
         input.message == 0x208 || input.message == 0x20b || input.message == 0x20c))
        original.input.parameter &= ~std::uint32_t(captured_mouse_buttons);
    if (input.message == 0x100 && is_typing_key(input.parameter) && interfaces.in_game() &&
        (interfaces.chat_active() || interfaces.console_visible()))
        return false;
    if (input.message != 0x100 && input.message != 0x201 && input.message != 0x204 &&
        input.message != 0x207 && input.message != 0x20b)
        return false;
    auto current_keys = keys;
    constexpr std::uint8_t mouse_keys[]{1, 2, 16, 17, 4, 5, 6};
    for (std::uint32_t i = 0; i < std::size(mouse_keys); ++i)
        if (captured_mouse_buttons & (1u << i))
            current_keys[mouse_keys[i] >> 5] &= ~(1u << (mouse_keys[i] & 31));
    for (std::uint32_t word = 0; word < current_keys.size(); ++word) {
        auto pressed = (previous_keys[word] ^ current_keys[word]) & current_keys[word];
        while (pressed) {
            const auto bit = static_cast<std::uint32_t>(std::countr_zero(pressed));
            const auto key = word * 32 + bit;
            if (key != 0 && key != 27 &&
                ((key != 1 && key != 2) || !(main_menu->input.mouse_flags & 1))) {
                if (hotkey_registry) {
                    for (auto* link = hotkey_registry->next; link && link != hotkey_registry;
                         link = link->next) {
                        auto* hotkey = static_cast<RegisteredHotkey*>(link);
                        if (hotkey->binding.packed >> 2 != key)
                            continue;
                        hotkey->binding.packed ^= 2;
                        if (hotkey->callback_object)
                            hotkey->callback(hotkey->callback_object, *hotkey);
                    }
                }
            }
            pressed &= ~(1u << bit);
        }
    }
    return false;
}
}

// Subcomponent: key_state.cpp

namespace skeet::input {
bool EngineInterfaces::in_game() const {
    return engine::invoke<bool>(engine_client, 26);
}
bool EngineInterfaces::console_visible() const {
    return engine::invoke<bool>(engine_client, 11);
}
bool EngineInterfaces::chat_active() const { // 0x43384AE8
    if (!find_hud_element)
        return false;
    auto* chat = find_hud_element(hud, "CCSGO_HudChat");
    return chat && engine::invoke<bool>(chat, 13);
}
bool is_typing_key(std::uint32_t key) noexcept { // 0x4334C872
    return key - 0x41 < 26 || key - 0x30 < 10 || key == 0x20 || key == 8 || key == 13 || key == 16;
}
void Runtime::set_key(std::uint32_t key, bool pressed) {
    // RtlSetBit/RtlClearBit ignore the bitmap's declared size. A valid Win32
    // virtual-key value is still required to fit the actual 256-bit storage.
    if (key >= keys.size() * 32)
        throw std::out_of_range("Virtual key exceeds input bitmap storage");
    const auto mask = std::uint32_t(1) << (key & 31);
    if (pressed)
        keys[key >> 5] |= mask;
    else
        keys[key >> 5] &= ~mask;
}
void Runtime::update_keys(const ui::InputMessage& message, bool consumed) { // 0x433B49FE
    if (message.message - 0x200 < 15) {
        mouse_buttons = static_cast<std::uint16_t>(message.parameter);
        std::uint32_t suppressed = 0;
        if (!consumed && interfaces.in_game() &&
            (interfaces.chat_active() || interfaces.console_visible()))
            suppressed = 7;
        constexpr std::uint8_t key_map[]{1, 2, 16, 17, 4, 5, 6};
        for (std::uint32_t i = 0; i < std::size(key_map); ++i) {
            const auto mask = std::uint32_t(1) << i;
            if (!(message.parameter & mask))
                set_key(key_map[i], false);
            else if (!(suppressed & mask))
                set_key(key_map[i], true);
        }
    } else if (message.message == 0x100) {
        if (is_typing_key(message.parameter) && interfaces.in_game() &&
            (interfaces.chat_active() || interfaces.console_visible()))
            return;
        if (message.parameter != 0 && message.parameter != 0x1b)
            set_key(message.parameter, true);
    } else if (message.message == 0x101)
        set_key(message.parameter & 0xff, false);
}
}

// Subcomponent: native_window_lifecycle.cpp
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

namespace skeet::input {
namespace {
constexpr char16_t mutex_name[] = u"$ IDA registry mutex $";
template <class Character>
std::uint32_t signature_hash(const Character* text) noexcept { //4334ADB0/43359C70
    std::uint32_t value = 0x811C9DC5;
    if (text)
        for (; *text; ++text)
            value = (value ^ static_cast<std::make_unsigned_t<Character>>(*text)) * 0x1000193u;
    return value ^ 0x92CA5DF1;
}
bool time_expired(std::int64_t now, std::int64_t then) noexcept {
    const auto difference = std::uint64_t(now) - std::uint64_t(then);
    const auto sign = std::uint64_t(0) - (difference >> 63);
    const auto magnitude = (difference ^ sign) - sign;
    // The native signed absolute value leaves INT64_MIN negative.
    return magnitude < (std::uint64_t(1) << 63) && magnitude > 10;
}
#if defined(_WIN32)
template <class Function> Function required_export(const char* library, const char* name) {
    const auto module = GetModuleHandleA(library);
    const auto function = module ? GetProcAddress(module, name) : nullptr;
    if (!function)
        throw std::runtime_error("Required native window-lifecycle export is missing");
    return reinterpret_cast<Function>(function);
}
std::uint32_t current_process_id() {
#if defined(_M_IX86) || defined(__i386__)
    return __readfsdword(0x20);
#else
    return static_cast<std::uint32_t>(__readgsqword(0x40));
#endif
}
#endif
}

NativeWindowLifecycleImports native_window_lifecycle_imports() {
#if defined(_WIN32)
    using Imports = NativeWindowLifecycleImports;
    return {required_export<Imports::Foreground>("user32.dll", "GetForegroundWindow"),
            required_export<Imports::OpenMutexFunction>("kernel32.dll", "OpenMutexW"),
            required_export<Imports::Close>("ntdll.dll", "ZwClose"),
            required_export<Imports::Delete>("ntdll.dll", "ZwDeleteFile"),
            required_export<Imports::Time>("msvcrt.dll", "_time64"),
            required_export<Imports::DefaultProcedure>("ntdll.dll", "NtdllDefWindowProc_W"),
            required_export<Imports::ModuleFileName>("kernel32.dll", "GetModuleFileNameA"),
            required_export<Imports::TempPath>("kernel32.dll", "GetTempPathW"),
            required_export<Imports::DosToNtPath>("ntdll.dll", "RtlDosPathNameToNtPathName_U"),
            required_export<Imports::FreeUnicode>("ntdll.dll", "RtlFreeUnicodeString"),
            required_export<Imports::Format>("msvcrt.dll", "sprintf_s"),
            required_export<Imports::AppendWide>("msvcrt.dll", "wcscat_s"),
            current_process_id};
#else
    throw std::runtime_error("Native window lifecycle requires Windows");
#endif
}

NativeWindowLifecycle::NativeWindowLifecycle(SettingsPersistence& persistence,
                                             NativeWindowLifecycleImports imports,
                                             engine::LoadedModuleEnvironment modules,
                                             engine::NativeImageEnvironment images)
    : persistence_(persistence), imports_(imports), modules_(modules), images_(images) {}

NativeWindowLifecycle::~NativeWindowLifecycle() { //433EBEDB
    if (dump_path.buffer)
        imports_.free_unicode(&dump_path);
}

std::int32_t NativeWindowLifecycle::delete_dump() {
    const NativeWindowObjectAttributes attributes{
        sizeof(NativeWindowObjectAttributes), 0, &dump_path, 0x40, nullptr, nullptr};
    return imports_.delete_file(&attributes);
}

void NativeWindowLifecycle::initialize() { //4348FD50
    auto* module =
        static_cast<std::byte*>(engine::find_loaded_module(modules_, 0xA42BE2F8, true, true));
    if (module) {
        class_name = reinterpret_cast<NativeWindowLifecycleImports::ClassName>(
            engine::resolve_pe_export(module, 0xC4A57DC2, 0, true, images_));
        foreground = reinterpret_cast<NativeWindowLifecycleImports::Foreground>(
            engine::resolve_pe_export(module, 0xDAA66BB8, 0, true, images_));
        internal_text = reinterpret_cast<NativeWindowLifecycleImports::InternalText>(
            engine::resolve_pe_export(module, 0x0306EBC1, 0, true, images_));
        window_list = reinterpret_cast<NativeWindowLifecycleImports::WindowList>(
            engine::resolve_pe_export(module, 0x087A9C92, 0, true, images_));
    }
    // Native initialization takes the second InLoadOrder module (ntdll).
    auto* native = reinterpret_cast<engine::LoaderModule*>(modules_.loader->modules.next->next);
    crc32 = reinterpret_cast<NativeWindowLifecycleImports::Crc32>(engine::resolve_pe_export(
        static_cast<std::byte*>(native->base), 0xA0681B30, 0, true, images_));

    std::array<std::uint32_t, 2> properties{0xFF123456, 0xFF123456};
    std::array<char, 260> executable{};
    if (imports_.module_file_name(0, executable.data(), executable.size())) {
        auto* separator = std::strrchr(executable.data(), '\\');
        if (!separator)
            separator = std::strrchr(executable.data(), '/');
        if (separator) {
            std::array<char, 128> title{};
            const auto process = imports_.current_process_id();
            imports_.format(title.data(), title.size(), "%s (%u) Properties", separator + 1,
                            process);
            properties[0] = signature_hash(title.data());
            imports_.format(title.data(), title.size(), "Eigenschaften von %s (%u)", separator + 1,
                            process);
            properties[1] = signature_hash(title.data());
        }
    }
    title_signatures = {0xB7774305, 0x402DF8B7, 0x7670079D,    0x8CA2613C,
                        0xFB81064D, 0x07EB0226, 0xC095ECA1,    0x613BACDE,
                        0xF6115A6E, 0x7ED6BAB9, properties[0], properties[1]};
    class_signatures = {0xDADC818D, 0x85A6C949, 0x2CF0DE78, 0xC4927E1E,
                        0x0A432040, 0x56AC0ADE, 0x28477FBE, 0x5178FBF8};
    paired_signatures = {{{0x8A6E61AA, 0xFEE2FC64}, {0xCA172992, 0x4F61CAF5}}};

    std::array<char16_t, 260> path{};
    imports_.temp_path(path.size(), path.data());
    imports_.append_wide(path.data(), path.size(), u"csgo.DMP");
    imports_.dos_to_nt_path(path.data(), &dump_path, nullptr, nullptr);
    path.fill(0);
    (void)delete_dump();
}

std::uint32_t NativeWindowLifecycle::scan() {
    if (const auto mutex = imports_.open_mutex(0x20000, 0, mutex_name)) {
        imports_.close(mutex);
        return 101;
    }
    if (deletion_pending && dump_path.buffer) {
        if (delete_dump() >= 0) {
            deletion_pending = false;
            return 200;
        }
        if (time_expired(imports_.time(nullptr), detection_time))
            deletion_pending = false;
    }
    if (!window_list)
        return 0;
    std::array<std::uintptr_t, 1024> windows;
    std::uint32_t count = 0;
    if (window_list(0, 0, 0, 0, 0, windows.size(), windows.data(), &count) < 0)
        return 0;
    for (std::uint32_t index = 0; index < count; ++index) {
        std::uint32_t class_hash = 0, title_hash = 0;
        std::array<char16_t, 1024> text{};
        if (class_name) {
            NativeWindowUnicodeString value{0, 0x800, text.data()};
            const auto length = class_name(windows[index], 0, &value);
            if (length >= 6) {
                class_hash = signature_hash(text.data());
                for (std::uint32_t match = 0; match < class_signatures.size(); ++match)
                    if (class_signatures[match] == class_hash)
                        return match + 1;
                if (length >= 19) {
                    text[18] = 0;
                    if (signature_hash(text.data()) == 0x690AB6B7)
                        return 8;
                }
            }
        }
        text.fill(0);
        if (imports_.default_window_procedure(windows[index], 0xD, text.size(),
                                              reinterpret_cast<std::intptr_t>(text.data())) >= 4) {
            title_hash = signature_hash(text.data());
            for (std::uint32_t match = 0; match < title_signatures.size(); ++match)
                if (title_signatures[match] == title_hash)
                    return match + 101;
            if (class_hash == pending_signature[0] && title_hash == pending_signature[1]) {
                deletion_pending = true;
                detection_time = imports_.time(nullptr);
            }
        }
        for (std::uint32_t match = 0; match < paired_signatures.size(); ++match)
            if (paired_signatures[match][0] == class_hash &&
                paired_signatures[match][1] == title_hash)
                return match + 201;
    }
    return 0;
}

void NativeWindowLifecycle::poll() { //433E291F
    const auto window = (foreground ? foreground : imports_.get_foreground_window)();
    if (!window)
        return;
    last_foreground = window;
    if (const auto reason = scan())
        persistence_.persist(reason, 0);
}
void NativeWindowLifecycle::poll_foreground_changed() { //4339879F, receiver is native owner+10
    const auto window = (foreground ? foreground : imports_.get_foreground_window)();
    if (!window || window == last_foreground)
        return;
    last_foreground = window;
    if (const auto reason = scan())
        persistence_.persist(reason, 0);
}

namespace {
#if defined(_M_IX86) || defined(__i386__)
#define WINDOW_MEMBER __thiscall
#define WINDOW_FAST __fastcall
#else
#define WINDOW_MEMBER
#define WINDOW_FAST
#endif
using OwnerEntry = void(WINDOW_MEMBER*)(void*);
struct WindowLifecycleOwner;
struct UpdateEndpoint : network::UpdateObserver {
    WindowLifecycleOwner* owner;
};
void WINDOW_MEMBER destroy_owner(void*);
void WINDOW_MEMBER initialize_owner(void*);
void WINDOW_MEMBER poll_owner(void*);
void WINDOW_MEMBER poll_changed(void*);
void WINDOW_MEMBER unchanged(void*) {} //433CCE8E
void WINDOW_FAST unchanged_player(void*, void*, std::uintptr_t, std::uintptr_t, std::uintptr_t) {
} //4337D653
void WINDOW_FAST unchanged_view(void*, float*, float*, float*) {} //4334A269
void WINDOW_FAST unchanged_post_view(void*, void*) {} //433CCE8E
const std::array<OwnerEntry, 9> owner_table{destroy_owner, initialize_owner, unchanged,
                                            unchanged,     unchanged,        unchanged,
                                            poll_owner,    unchanged,        unchanged};
const network::UpdateObserverVtable update_table{
    {poll_changed, unchanged, unchanged, unchanged, unchanged},
    unchanged_player,
    unchanged_view,
    unchanged_post_view};
struct WindowLifecycleOwner {
    const OwnerEntry* vtable = nullptr;
    std::uintptr_t reserved = 0;
    client::NativeCommandPhaseReceiver command;
    UpdateEndpoint update{{nullptr}, this};
    NativeWindowLifecycle runtime;
    engine::JobMemory memory;
    WindowLifecycleOwner(engine::OwnerRegistryBinding owners,
                         client::CommandPhaseRegistry& commands,
                         engine::OwnerRegistryBinding updates, SettingsPersistence& persistence,
                         NativeWindowLifecycleImports imports,
                         engine::LoadedModuleEnvironment modules,
                         engine::NativeImageEnvironment images)
        : runtime(persistence, imports, modules, images), memory(owners.memory) {
        owners.append(this); //433676A0
        commands.append(command, &runtime); //43345EBC, canonical434757A0 storage
        network::register_update_observer(updates, update); //433B8C1C
        vtable = owner_table.data();
        commands.publish(command, client::CommandOwnerId::owner_4346eed8); //4347F584
        update.vtable = &update_table;
    }
};
void WINDOW_MEMBER destroy_owner(void* object) { //43387ABC ->433EBEDB ->433837D0
    auto* owner = static_cast<WindowLifecycleOwner*>(object);
    const auto memory = owner->memory;
    owner->~WindowLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void WINDOW_MEMBER initialize_owner(void* object) {
    static_cast<WindowLifecycleOwner*>(object)->runtime.initialize();
}
void WINDOW_MEMBER poll_owner(void* object) {
    static_cast<WindowLifecycleOwner*>(object)->runtime.poll();
}
void WINDOW_MEMBER poll_changed(void* object) {
    static_cast<UpdateEndpoint*>(object)->owner->runtime.poll_foreground_changed();
}
#undef WINDOW_MEMBER
#undef WINDOW_FAST
}

std::size_t native_window_lifecycle_owner_size() noexcept {
    return sizeof(WindowLifecycleOwner);
}
NativeWindowLifecycle& construct_native_window_lifecycle(
    void* storage, engine::OwnerRegistryBinding owners, client::CommandPhaseRegistry& commands,
    engine::OwnerRegistryBinding updates, SettingsPersistence& persistence,
    NativeWindowLifecycleImports imports, engine::LoadedModuleEnvironment modules,
    engine::NativeImageEnvironment images) {
    auto* owner = ::new (storage)
        WindowLifecycleOwner(owners, commands, updates, persistence, imports, modules, images);
    return owner->runtime;
}
} // namespace skeet::input

// Subcomponent: window_hook.cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::input {
namespace {
WindowSubclassRuntime* active_runtime = nullptr;
template <class Function> Function window_hook_required_export(const char* module_name, const char* name) {
    const auto module = GetModuleHandleA(module_name);
    const auto function = module ? GetProcAddress(module, name) : nullptr;
    if (!function)
        throw std::runtime_error("Required window-hook system export is missing");
    return reinterpret_cast<Function>(function);
}
std::uint64_t timestamp_counter() {
    return __rdtsc();
}
std::uintptr_t* thread_id_slot() {
    using CurrentTeb = void*(WINAPI*)();
    static const auto current = window_hook_required_export<CurrentTeb>("ntdll.dll", "NtCurrentTeb");
    auto* const teb = static_cast<std::byte*>(current());
    return reinterpret_cast<std::uintptr_t*>(teb + (sizeof(void*) == 4 ? 0x24 : 0x48));
}
std::uint32_t __stdcall window_thread(std::uintptr_t window, std::uint32_t* process_id) {
    DWORD process = 0;
    const auto thread =
        GetWindowThreadProcessId(reinterpret_cast<HWND>(window), process_id ? &process : nullptr);
    if (process_id)
        *process_id = process;
    return thread;
}
}
WindowHookImports native_window_hook_imports(const engine::CommonControlsExports& common_controls) {
    if (!common_controls.set_window_subclass || !common_controls.default_subclass_proc)
        throw std::runtime_error("Common-controls startup imports are incomplete");
    return {timestamp_counter,
            thread_id_slot,
            reinterpret_cast<WindowHookImports::SetSubclass>(common_controls.set_window_subclass),
            reinterpret_cast<WindowHookImports::DefaultProcedure>(
                common_controls.default_subclass_proc),
            window_thread,
            window_hook_required_export<WindowHookImports::FileHeader>("ntdll.dll", "RtlPcToFileHeader")};
}
void WindowSubclassRuntime::install(std::uintptr_t window) { // 0x433CA545
    auto& state = hooks.state.window;
    state.window = window;
    state.subclass_id = (imports.read_timestamp_counter() % 0x7fff) | 1u;
    state.restored_ansi_procedure = hooks.system.get_window_long_a(window, -4);
    state.restored_wide_procedure = hooks.system.get_window_long_w(window, -4);
    const auto thread = imports.get_window_thread(window, nullptr);
    auto* const current_thread = imports.current_thread_id_slot();
    const auto previous_thread = *current_thread;
    if (thread != previous_thread)
        *current_thread = thread;
    const auto procedure = pending_procedure;
    pending_procedure = nullptr;
    const auto installed = imports.set_window_subclass(
        window, procedure, state.subclass_id, std::uint32_t(state.restored_wide_procedure));
    if (thread != previous_thread)
        *current_thread = previous_thread;
    if (installed && hooks.system.get_window_long_a(window, -4) == state.restored_ansi_procedure &&
        hooks.system.get_window_long_w(window, -4) == state.restored_wide_procedure) {
        const auto candidate = std::uint32_t(hooks.system.get_window_long_w(state.window, -21));
        if (candidate) {
            const auto common_controls =
                engine::find_loaded_module(modules, 0xc0ba92c9, true, true);
            if (common_controls) {
                void* base = nullptr;
                if (imports.pc_to_file_header(reinterpret_cast<void*>(std::uintptr_t(candidate)),
                                              &base) &&
                    base == common_controls)
                    hooks.system.set_window_long_w(state.window, -4, std::int32_t(candidate));
            }
        }
    }
    state.expected_ansi_procedure = hooks.system.get_window_long_a(window, -4);
    state.expected_wide_procedure = hooks.system.get_window_long_w(window, -4);
    if (!installed) {
        state.window = 0;
        state.subclass_id = 0;
    }
}
std::intptr_t WindowSubclassRuntime::process(std::uintptr_t window, std::uint32_t message,
                                             std::uintptr_t parameter, std::intptr_t key_data,
                                             std::uintptr_t identifier) { // 0x4341F1DF
    if (identifier == hooks.state.window.subclass_id) {
        if (message == 0x82)
            hooks.restore_window();
        else {
            ui::WindowMessage input_message{
                window, {message, std::uint32_t(parameter), std::uint32_t(key_data)}};
            if (input.dispatch(input_message))
                return 1;
            parameter = input_message.input.parameter;
            key_data = std::int32_t(input_message.input.key_data);
        }
    }
    return imports.default_subclass_proc(window, message, parameter, key_data);
}
void bind_window_subclass_runtime(WindowSubclassRuntime& runtime) noexcept {
    active_runtime = &runtime;
    runtime.hooks.state.window.subclass_procedure =
        reinterpret_cast<void*>(window_subclass_callback);
}
std::intptr_t __stdcall window_subclass_callback(void* window, std::uint32_t message,
                                                 std::uintptr_t parameter, std::intptr_t key_data,
                                                 std::uintptr_t identifier, std::uintptr_t) {
    return active_runtime->process(reinterpret_cast<std::uintptr_t>(window), message, parameter,
                                   key_data, identifier);
}
}
