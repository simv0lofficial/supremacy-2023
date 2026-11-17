#pragma once
#include "skeet/misc/native_text.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include <string_view>

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;
}
namespace skeet::script {

// View the retained image's fifteen native16-byte strings at43479670..4347975F.
// Binding does not reset, relocate or copy the fields. Ordinary CRT433E495A
// performs initialization; Lua and the retained VM read those same bytes.
class UiTypeStorage {
public:
    UiTypeStorage(engine::PreservedInitialization&, engine::JobMemory);
    UiTypeStorage(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    UiTypeStorage(const UiTypeStorage&) = delete;
    UiTypeStorage& operator=(const UiTypeStorage&) = delete;
    UiTypeStorage(UiTypeStorage&&) = delete;
    UiTypeStorage& operator=(UiTypeStorage&&) = delete;

    misc::NativeText* initialize(); //43484040, returns the first native string
    void destroy(); //433C04BA ->433FF75D, reverse order; fields left unchanged
    const misc::NativeText& at(std::size_t kind) const {
        return names_.at(kind);
    }
    std::string_view name(std::size_t kind) const;
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    engine::JobMemory memory() const noexcept {
        return memory_;
    }

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    std::array<misc::NativeText, 15>& names_;
    engine::JobMemory memory_;
};

// The same context must outlive both startup and the process exit registry.
// Storage is borrowed by UiValues::type and is never copied into a Lua module.
struct UiTypeInitialization {
    UiTypeStorage& storage;
    client::ExitCallbacks& callbacks;
    UiTypeInitialization(UiTypeStorage& storage, client::ExitCallbacks& callbacks) noexcept
        : storage(storage), callbacks(callbacks) {}
    UiTypeInitialization(const UiTypeInitialization&) = delete;
    UiTypeInitialization& operator=(const UiTypeInitialization&) = delete;
    UiTypeInitialization(UiTypeInitialization&&) = delete;
    UiTypeInitialization& operator=(UiTypeInitialization&&) = delete;
    const engine::NativeDataImage& image() const noexcept {
        return storage.image();
    }
};

// One individual ordinary operand230, not a group/static-window initializer.
void bind_ui_type_initializer(client::NativeStartupSources&, UiTypeInitialization&);
void finalize_ui_type_storage(); //actual no-argument source callback433C04BA
} //namespace skeet::script
