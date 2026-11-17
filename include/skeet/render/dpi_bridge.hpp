#pragma once

namespace skeet::ui {
struct GuiDpiState;
class RendererContainer;

// The live GUI owner binds renderer front slot80 to its concrete scale setter.
// Both objects outlive this bridge; font rasterization remains deferred until
// RendererContainer's normal frame consumer observes the requested scale.
class RendererDpiBridge final {
public:
    RendererDpiBridge(GuiDpiState& dpi, RendererContainer& renderer) noexcept;
    ~RendererDpiBridge();
    RendererDpiBridge(const RendererDpiBridge&) = delete;
    RendererDpiBridge& operator=(const RendererDpiBridge&) = delete;
    RendererDpiBridge(RendererDpiBridge&&) = delete;
    RendererDpiBridge& operator=(RendererDpiBridge&&) = delete;

    void refresh() noexcept;

private:
    static void refresh_bound_renderer(void*) noexcept;
    GuiDpiState& dpi_;
    RendererContainer& renderer_;
};
}
