#include "skeet/script/ui_type_storage.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/client/shutdown.hpp"
#include <stdexcept>

namespace skeet::script {
namespace {
UiTypeInitialization* active_initialization = nullptr;

void initialize_types_and_register(void* owner) { //433E495A
    auto& context = *static_cast<UiTypeInitialization*>(owner);
    // One no-argument callback represents one native global array. Reject a
    // second context before touching storage; exit append deduplicates callbacks.
    if (active_initialization && active_initialization != &context)
        throw std::logic_error("UI type exit callback requires one stable storage binding");
    context.storage.initialize();
    active_initialization = &context;
    (void)context.callbacks.append(finalize_ui_type_storage);
}
}

UiTypeStorage::UiTypeStorage(engine::PreservedInitialization& initialization,
                             engine::JobMemory memory)
    : UiTypeStorage(initialization.image_owner(), memory) {}
UiTypeStorage::UiTypeStorage(std::shared_ptr<engine::NativeDataImage> image,
                             engine::JobMemory memory)
    : image_(image),
      names_(
          engine::require_native_data_image(image_).native_cell<std::array<misc::NativeText, 15>>(
              0x43479670u)),
      memory_(memory) {}

misc::NativeText* UiTypeStorage::initialize() {
    // Decoded immediate strings in complete43484040, in its actual call order.
    // Each call initializes the native three-pointer prefix and allocates its
    // own text through433B2213/433C1360. No separate lookup catalog is retained.
    misc::construct_native_text(names_[0], "unknown", memory_);
    misc::construct_native_text(names_[1], "tab", memory_);
    misc::construct_native_text(names_[2], "button", memory_);
    misc::construct_native_text(names_[3], "checkbox", memory_);
    misc::construct_native_text(names_[4], "combobox", memory_);
    misc::construct_native_text(names_[5], "slider", memory_);
    misc::construct_native_text(names_[6], "listbox", memory_);
    misc::construct_native_text(names_[7], "group", memory_);
    misc::construct_native_text(names_[8], "label", memory_);
    misc::construct_native_text(names_[9], "hotkey", memory_);
    misc::construct_native_text(names_[10], "textbox", memory_);
    misc::construct_native_text(names_[11], "scrollbar", memory_);
    misc::construct_native_text(names_[12], "color_picker", memory_);
    misc::construct_native_text(names_[13], "multiselect", memory_);
    misc::construct_native_text(names_[14], "context_menu", memory_);
    return names_.data();
}

void UiTypeStorage::destroy() {
    for (std::size_t kind = names_.size(); kind;)
        misc::destroy_native_text(names_[--kind], memory_);
}

std::string_view UiTypeStorage::name(std::size_t kind) const {
    const auto& text = at(kind);
    // Pointer-word subtraction matches4341F0BD and also permits the original
    // zero-initialized array to supply(nullptr,0) before its ordinary initializer.
    const auto length =
        reinterpret_cast<std::uintptr_t>(text.end) - reinterpret_cast<std::uintptr_t>(text.begin);
    return {text.begin, length};
}

void finalize_ui_type_storage() {
    active_initialization->storage.destroy();
}

void bind_ui_type_initializer(client::NativeStartupSources& sources,
                              UiTypeInitialization& context) {
    if (&context.image() != &sources.data())
        throw std::logic_error("UI type initializer must use the startup canonical data owner");
    sources.bind_initializer({0x433E495Au, &context, initialize_types_and_register});
}
} //namespace skeet::script
