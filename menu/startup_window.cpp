// Main Menu Window Lifetime & Startup
#include "util/address.h"
using skeet::field;
using skeet::store;


// Subcomponent: startup_window_name.cpp
#include "skeet/menu/startup_window_name.hpp"
#include <algorithm>
#include <stdexcept>
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#endif

namespace skeet::ui {
std::u16string_view MainMenuRegistryName::view() const {
    const auto end = std::find(text.begin(), text.end(), char16_t{});
    if (end == text.end())
        throw std::logic_error("Main menu registry name has no native terminator");
    return {text.data(), std::size_t(end - text.begin())};
}
MainMenuCpuLeaf1 read_main_menu_cpu_leaf1() {
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    int words[4];
    __cpuidex(words, 1, 0);
    return {static_cast<std::uint32_t>(words[0]), static_cast<std::uint32_t>(words[1]),
            static_cast<std::uint32_t>(words[2]), static_cast<std::uint32_t>(words[3])};
#elif defined(__i386__) || defined(__x86_64__)
    MainMenuCpuLeaf1 result;
    __cpuid_count(1, 0, result.eax, result.ebx, result.ecx, result.edx);
    return result;
#else
    throw std::logic_error("Main menu registry naming requires the original x86 CPUID input");
#endif
}
MainMenuRegistryName make_main_menu_registry_name(MainMenuCpuLeaf1 cpu) noexcept {
    // The final startup CPUID at434AF5E2 feeds the unsigned value formatted
    // by434C7805 into native menu+E0. The addition wraps before the shift.
    auto value = main_menu_registry_value(cpu);
    std::array<char16_t, 10> reversed;
    std::size_t count = 0;
    do {
        reversed[count++] = char16_t(u'0' + value % 10);
        value /= 10;
    } while (value);
    MainMenuRegistryName result;
    for (std::size_t index = 0; index < count; ++index)
        result.text[index] = reversed[count - index - 1];
    return result;
}
MainMenuRegistryName make_main_menu_registry_name() {
    return make_main_menu_registry_name(read_main_menu_cpu_leaf1());
}
} //namespace skeet::ui

// Subcomponent: window_lifetime.cpp
#include "skeet/menu/window_lifetime.hpp"
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::ui {
WindowLifetimeImports native_window_lifetime_imports() {
    const auto native = GetModuleHandleA("ntdll.dll");
    const auto release = native ? GetProcAddress(native, "RtlFreeUnicodeString") : nullptr;
    if (!release)
        throw std::runtime_error("Required RtlFreeUnicodeString export is missing");
    return {reinterpret_cast<WindowLifetimeImports::FreeUnicodeString>(release)};
}
void WindowCallbackStorage::initialize(MenuWindow& window) {
    const auto registrations = native_window_handlers();
    handlers_.assign(registrations.begin(), registrations.end());
    handler_buckets_.assign(12, nullptr);
    handler_buckets_.back() = &handler_end_;
    for (auto& handler : handlers_) {
        const auto bucket = handler.key % 11;
        handler.next = handler_buckets_[bucket];
        handler_buckets_[bucket] = &handler;
    }
    notification_buckets_ = {nullptr, &notification_end_};
    window.message_handlers = handler_buckets_;
    window.notifications = notification_buckets_;
}

void WindowCallbackStorage::destroy(MenuWindow& window) noexcept {
    window.message_handlers = {};
    std::vector<WindowMessageHandler*>{}.swap(handler_buckets_);
    std::vector<WindowMessageHandler>{}.swap(handlers_);
    window.notifications = {};
    notification_buckets_[0] = nullptr;
}

void initialize_menu_window(MenuWindow& window, WindowCallbackStorage& callbacks,
                            std::uintptr_t native_window, Point size) { // 0x43485772
    window.native_window = native_window;
    window.next = nullptr;
    window.position = {};
    window.default_size = size;
    window.geometry.size = size;
    window.geometry.start_padding = {7, 10};
    window.geometry.end_padding = {7, 6};
    window.input.mouse_flags = 1;
    window.button_flags = 0;
    window.cursor = {};
    window.tabs.clear();
    // Initialize the shared widths before any category constructs controls.
    // GuiDpiState starts at1.0, so refresh(false) at100% does not run the DPI
    // callback and cannot initialize an otherwise-zero SliderMetrics object.
    window.update_dpi(1.0f);
    window.navigation.active_tab = 0;
    window.transition_started = window.read_ticks(window.clock_context);
    window.transition_state = 2;
    window.opacity = 1.0f;
    callbacks.initialize(window);
}

void destroy_menu_window(MenuWindow& window,
                         WindowCallbackStorage& callbacks) noexcept { // 0x4333F611
    callbacks.destroy(window);
    window.tab_context_menu = nullptr;
    for (auto*& tab : window.tabs) {
        delete tab;
        tab = nullptr;
    }
    std::vector<Tab*>{}.swap(window.tabs);
}
MenuWindow* delete_menu_window(MenuWindow* window, WindowCallbackStorage& callbacks,
                               std::uint8_t flags) noexcept { // 0x43328FEB
    destroy_menu_window(*window, callbacks);
    if (flags & 1)
        delete window;
    return window;
}
void destroy_main_menu_window(MenuWindow& window, WindowCallbackStorage& callbacks,
                              WindowTitleString& title,
                              const WindowLifetimeImports& imports) noexcept { // 0x433E71FA
    if (title.buffer) {
        imports.free_unicode_string(&title);
        title = {};
    }
    destroy_menu_window(window, callbacks);
}
MenuWindow* delete_main_menu_window(MenuWindow* window, WindowCallbackStorage& callbacks,
                                    WindowTitleString& title, const WindowLifetimeImports& imports,
                                    std::uint8_t flags) noexcept { // 0x4334C6CB
    destroy_main_menu_window(*window, callbacks, title, imports);
    if (flags & 1)
        delete window;
    return window;
}
}

// Subcomponent: startup_window.cpp
#include "skeet/menu/startup_window.hpp"
#include "skeet/menu/startup_data.hpp"
#include "skeet/menu/root_menu.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/engine/build_startup_convars.hpp"
#include "skeet/rage/group_storage.hpp"
#include "skeet/render/startup_renderer.hpp"
#include "skeet/script/owner.hpp"
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::ui {
namespace {
std::uintptr_t __stdcall find_game_window(const char* name, const char* title) {
    return reinterpret_cast<std::uintptr_t>(FindWindowA(name, title));
}
}
MainMenuWindowImports native_main_menu_window_imports() {
    return {find_game_window};
}

MainMenuWindowOwner::MainMenuWindowOwner(client::ClientRuntimeStorage& storage)
    : MainMenuWindowOwner(storage, native_main_menu_window_imports()) {}

MainMenuWindowOwner::MainMenuWindowOwner(client::ClientRuntimeStorage& storage,
                                         MainMenuWindowImports imports)
    : storage_(storage), window_(storage.menu_startup_data().window_events()) {
    auto& input = storage_.input_runtime();
    auto& head = storage_.data().native_cell<MenuWindow*>(0x4347691Cu);
    auto& dpi = storage_.menu_startup_data().dpi;
    if (head || input.main_menu || input.windows || dpi.windows)
        throw std::logic_error("Main menu startup requires its unconstructed process window list");
    if (!imports.find_window)
        throw std::invalid_argument("Main menu startup requires the genuine FindWindowA import");
    const auto handle =
        imports.find_window("Valve001", "Counter-Strike: Global Offensive - Direct3D 9");
    if (!handle)
        throw std::runtime_error("The main game window was not found");
    initialize_menu_window(window_, callbacks_, handle, initial_size);
    window_alive_ = true;
    input.main_menu = &window_;
    input.windows = &window_;
    dpi.windows = &window_;
    head = &window_;
}
MainMenuWindowOwner::~MainMenuWindowOwner() {
    release_controller();
    release_window_views();
    if (window_alive_)
        destroy_menu_window(window_, callbacks_);
}
void MainMenuWindowOwner::release_controller() noexcept {
    auto*& current_controller = storage_.data().native_cell<RootMenu*>(0x43476924u);
    if (controller_ && current_controller == controller_.get())
        current_controller = nullptr;
    controller_.reset();
}
void MainMenuWindowOwner::release_window_views() noexcept {
    auto& input = storage_.input_runtime();
    auto& head = storage_.data().native_cell<MenuWindow*>(0x4347691Cu);
    auto& dpi = storage_.menu_startup_data().dpi;
    if (input.main_menu == &window_)
        input.main_menu = nullptr;
    if (input.windows == &window_)
        input.windows = nullptr;
    if (dpi.windows == &window_)
        dpi.windows = nullptr;
    if (head == &window_)
        head = nullptr;
}
void MainMenuWindowOwner::release_gui(WindowTitleString& title,
                                      const WindowLifetimeImports& imports) noexcept {
    release_controller();
    if (window_alive_) {
        destroy_main_menu_window(window_, callbacks_, title, imports);
        window_alive_ = false;
    }
    release_window_views();
}
MenuStartupData& MainMenuWindowOwner::menu_data() const noexcept {
    return storage_.menu_startup_data();
}
RootMenu& MainMenuWindowOwner::compose(MainMenuFeatureOwners owners, StartupRenderer& renderer,
                                       const engine::BuildStartupConVarBindings& variables) {
    if (controller_ || storage_.data().native_cell<RootMenu*>(0x43476924u))
        throw std::logic_error("Main menu controller has already been constructed");
    registry_name_ = make_main_menu_registry_name();
    auto& menu = storage_.menu_startup_data();
    auto& config = owners.configuration;
    if (&config.native_storage() != &storage_.config_storage() ||
        &config.configuration() != &storage_.configuration() ||
        &owners.rage_groups.image() != &storage_.data() || &variables.data() != &storage_.data() ||
        &renderer.dpi() != &menu.dpi ||
        storage_.data().native_cell<RendererContainer*>(0x4347690Cu) != &renderer.renderer())
        throw std::invalid_argument("Main menu composition requires its canonical process owners");
    auto& shutdown = owners.scripts.modules().shutdown_state();
    const auto baseline_a =
        ConfigStreamView::native_vector(config.native_storage().baseline_cells());
    const auto baseline_b = ConfigStreamView::script_state(shutdown);
    auto* const engine = storage_.data().native_cell<void*>(0x4346A514u);
    RootMenuBindings bindings{config.configuration(),
                              owners.identities,
                              owners.rage_groups,
                              baseline_a,
                              baseline_b,
                              config.clipboard(),
                              owners.camera,
                              owners.miscellaneous,
                              storage_.settings_menu_storage().values(),
                              owners.settings_runtime,
                              storage_.data().native_cell<std::uint8_t>(0x43478C6Cu),
                              storage_.data().native_cell<std::uint8_t>(0x434758D4u) != 0,
                              owners.skins_catalog,
                              owners.skins_refresh,
                              owners.players,
                              owners.player_bindings,
                              config.presets(),
                              menu.script_values(),
                              owners.scripts,
                              owners.icons,
                              menu.dpi,
                              renderer.renderer(),
                              menu.root_globals(),
                              engine,
                              variables.accuracy_variable().get(),
                              storage_.input_runtime()};
    auto result = std::make_unique<RootMenu>(window_, bindings);
    storage_.config_storage().bind_source_menu_registry(*result, registry_name_);
    controller_ = std::move(result);
    storage_.data().native_cell<RootMenu*>(0x43476924u) = controller_.get();
    config.bind_menu(*controller_, owners.scripts.modules().manager());
    return *controller_;
}
RootMenu& MainMenuWindowOwner::controller() const {
    if (!controller_)
        throw std::logic_error("Main menu controller has not been composed");
    return *controller_;
}
} //namespace skeet::ui
