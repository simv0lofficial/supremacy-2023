#pragma once
#include "backend_container.hpp"
#include "device_listener.hpp"
#include "frame_runtime.hpp"
#include "skeet/engine/native_data_image.hpp"
#include <span>
#include <optional>

namespace skeet::ui {
struct GuiDpiState;
struct StartupFontRecipe {
    std::u16string_view face;
    std::int32_t height;
    std::uint32_t flags;
};
// Initial GDI fonts made by4348F2C1. The later definition-only edits are
// preserved by initialize_startup_fonts, rather than rebuilding these fonts.
std::span<const StartupFontRecipe, 7> startup_font_recipes() noexcept;
void initialize_startup_fonts(RendererContainer&);

struct BuildRendererBindings {
    IDirect3DDevice9** device_slot; // borrowed shader-device state
    void** bridge_holder; // nullable Steam CD3D9Renderer* slot
};
// Actual loaded module/interface inputs, with build-specific instruction and
// section provenance checks. No captured device/object pointer is an input.
BuildRendererBindings discover_build_renderer_bindings(const engine::NativeDataImage&);

// Concrete renderer lifetime for the source bootstrap. Its object is larger
// than the native84-byte front, so source consumers share this one C++ object
// through4347690C. The adjacent native menu BSS is never overwritten.
// RootMenu owns the DPI bridge; it borrows this same renderer and DPI object.
class StartupRenderer {
public:
    StartupRenderer(std::shared_ptr<engine::NativeDataImage>, GuiDpiState&);
    StartupRenderer(std::shared_ptr<engine::NativeDataImage>, GuiDpiState&,
                    RendererDeviceListener&);
    ~StartupRenderer();
    StartupRenderer(const StartupRenderer&) = delete;
    StartupRenderer& operator=(const StartupRenderer&) = delete;
    RendererContainer& renderer() noexcept {
        return renderer_;
    }
    RendererDeviceListener& device_listener() noexcept {
        return *listener_;
    }
    FrameRuntime& frame() noexcept {
        return frame_;
    }
    GuiDpiState& dpi() const noexcept {
        return dpi_;
    }

private:
    std::shared_ptr<engine::NativeDataImage> data_;
    GuiDpiState& dpi_;
    BuildRendererBindings bindings_;
    RendererContainer renderer_;
    std::optional<RendererDeviceListener> owned_listener_;
    RendererDeviceListener* listener_;
    FrameRuntime frame_;
    void initialize();
};
}
