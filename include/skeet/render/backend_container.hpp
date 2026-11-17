#pragma once
#include "backend.hpp"
#include "font_runtime.hpp"
#include <array>
#include <memory>
#include <mutex>
#include <string>

namespace skeet::ui {

struct FontDefinition {
    std::u16string face;
    std::int32_t height = 0;
    std::uint32_t flags = 0;
};

// The 0x84-byte native front object at 4347690C has four command vectors and
// a refcounted published-frame guard. The D3D/font context is a separate owner.
class RendererContainer {
public:
    explicit RendererContainer(IDirect3DDevice9* device);
    ~RendererContainer();
    RendererContainer(const RendererContainer&) = delete;
    RendererContainer& operator=(const RendererContainer&) = delete;
    FontBackend fonts;
    std::array<FontDefinition, 7> font_definitions;
    std::array<std::unique_ptr<FontRuntime>, 7> default_fonts, scaled_fonts;
    RendererBackend& backend() noexcept {
        return *backend_;
    }
    RendererBackend* backend_if_alive() noexcept {
        return backend_.get();
    }
    void configure_fonts(std::span<const FontDefinition, 7> definitions);
    void set_font_scale(float scale) noexcept;
    float requested_font_scale() const noexcept;
    RenderQueue* begin(); // 4339E884
    void publish(); // 434183CF
    bool draw(); // 4338ACAE
    void release_context(); // 433F529D / 4340AF6A, preserves the front queues
    void destroy(); // 433E938B / 4336E54C / 4340AF6A
private:
    struct Frame {
        std::atomic<std::uint32_t> state{0};
        RenderQueue queue;
    };
    struct PublishedFrame {
        explicit PublishedFrame(Frame& value) noexcept : frame(&value) {}
        ~PublishedFrame() {
            frame->state.exchange(0, std::memory_order_acq_rel);
        } // 433F27B1
        Frame* frame;
    };
    std::array<Frame, 4> frames_;
    Frame* active_ = nullptr;
    std::shared_ptr<PublishedFrame> published_;
    void refresh_fonts();
    SRWLOCK font_lock_ = SRWLOCK_INIT;
    std::atomic<float> requested_scale_{1.0f};
    bool fonts_configured_ = false;
    std::unique_ptr<RendererBackend> backend_;
};
} // namespace skeet::ui
