#pragma once
#include "backend_container.hpp"
#include <variant>

namespace skeet::ui {
// Native object 43468B7C, three-entry vtable 4346FD7C. The device manager's
// slots 20/21 register/unregister this same borrowed object. The renderer must
// outlive that registration; its backend may be destroyed in the meantime.
// No virtual destructor: the listener's first slot is device-object release.
class RendererDeviceListener {
public:
    explicit RendererDeviceListener(RendererContainer& renderer) noexcept : renderer_(&renderer) {}
    // Native43468B7C exists before4347690C is published. It follows that live
    // slot through renderer creation and teardown; no renderer is fabricated.
    explicit RendererDeviceListener(RendererContainer* const& renderer) noexcept
        : renderer_(&renderer) {}
    bool follows(RendererContainer* const& renderer) const noexcept;
    void restore_base_vtable() noexcept;
    virtual void release_device_objects(); // 4334825B, tail branch to 433C9479
    virtual void set_device(IDirect3DDevice9*, std::uintptr_t, std::uintptr_t); // 433874F9, ret12
    virtual void notification(std::uintptr_t, std::uintptr_t); // 4334A269, ret8
private:
    using Binding = std::variant<RendererContainer*, RendererContainer* const*>;
    explicit RendererDeviceListener(Binding binding) noexcept : renderer_(binding) {}
    RendererContainer* renderer() const noexcept;
    Binding renderer_;
};
} // namespace skeet::ui
