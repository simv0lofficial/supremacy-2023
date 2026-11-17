#pragma once
#include "skeet/render_queue.hpp"
#include <d3d9.h>
#include <span>
#include <vector>
#include <atomic>

namespace skeet::ui {
class FontRuntime;

// Native FVF 0x142: XYZ, D3DCOLOR, TEX1. No RHW or half-pixel correction.
struct RenderVertex {
    float x, y, z;
    std::uint32_t color;
    float u, v;
};
static_assert(sizeof(RenderVertex) == 24);
struct ColoredVertex {
    float x, y;
    std::uint32_t color;
};
static_assert(sizeof(ColoredVertex) == 12);

struct TextureRecord {
    std::uint32_t key = 0;
    std::int32_t width = 0, height = 0;
    std::vector<std::byte> pixels;
    std::uint32_t timestamp = 0;
    IDirect3DTexture9* texture = nullptr;
    bool dirty = true;
};

class RendererBackend {
    friend class RendererContainer;

public:
    explicit RendererBackend(IDirect3DDevice9* device);
    ~RendererBackend();
    RendererBackend(const RendererBackend&) = delete;
    RendererBackend& operator=(const RendererBackend&) = delete;

    IDirect3DDevice9* device() const noexcept {
        return device_;
    }
    void set_device(IDirect3DDevice9* device) noexcept {
        device_ = device;
    } // 433874F9
    std::uint32_t vertex_capacity() const noexcept {
        return capacity_;
    }
    std::uint32_t pending_vertices() const noexcept {
        return count_;
    }
    std::uint32_t first_vertex() const noexcept {
        return first_;
    }
    D3DPRIMITIVETYPE primitive() const noexcept {
        return primitive_;
    }
    IDirect3DBaseTexture9* bound_texture() const noexcept {
        return texture_;
    }
    float alpha() const noexcept {
        return alpha_;
    }
    float depth() const noexcept {
        return depth_;
    }
    std::int32_t screen_width = 0, screen_height = 0;
    FontBackend* fonts = nullptr;

    // Frame setup/restoration: 433C95CE / 4337EF22. The caller supplies the
    // device already used by the game's render thread.
    bool begin_frame();
    void end_frame();
    void release_device_objects(); // 433C9479
    void wait_for_render(); // 4340AF6A, bounded shutdown wait
    void flush(bool reset_start = false); // 433B18B1
    bool append_vertices(D3DPRIMITIVETYPE primitive, IDirect3DBaseTexture9* texture,
                         std::span<const RenderVertex> vertices, bool connected = false);
    void rectangle(RectangleCommand command);
    void four_color_gradient(FourColorGradientCommand command);
    void vertical_gradient(GradientCommand command);
    void horizontal_gradient(GradientCommand command);
    void line(LineCommand command);
    void polyline(std::span<const LinePoint> points, std::uint32_t color);
    void triangle(TriangleCommand command);
    void triangle_list(std::span<const ColoredVertex> vertices);
    void triangle_strip(std::span<const ColoredVertex> vertices);
    void colored_primitives(std::uint32_t type, std::span<const ColoredVertex> vertices);
    void circle(CircleCommand command);
    void circle_outline(CircleOutlineCommand command);
    void texture(TextureCommand command);
    void set_bounds(const std::array<std::int32_t, 4>& bounds);
    std::array<std::int32_t, 4> get_bounds() const;
    void reset_bounds(bool enabled);
    bool is_bounds_reset() const;
    void set_alpha(float value);
    void set_depth(float value) noexcept;

    // Executes the original byte stream, including absolute end offsets.
    // Text uses the concrete font cache bound to fonts.
    void consume(std::span<const std::byte> commands);
    bool render(const RenderQueue& queue);
    void text(std::int32_t x, std::int32_t y, std::uint32_t color, std::uint32_t flags,
              std::int32_t maximum_width, std::u16string_view text);

    std::uint32_t create_texture(std::uint32_t key, std::int32_t width, std::int32_t height,
                                 std::span<const std::byte> pixels);
    void update_texture(std::uint32_t index, std::int32_t width, std::int32_t height,
                        std::span<const std::byte> pixels);
    std::int32_t find_texture(std::uint32_t key) const noexcept;
    void texture_size(std::uint32_t index, std::int32_t* width,
                      std::int32_t* height) const noexcept;
    std::span<const TextureRecord> textures() const noexcept {
        return textures_;
    }
    void upload_textures();
    void upload_fonts();
    HRESULT upload_texture(std::int32_t width, std::int32_t height,
                           std::span<const std::byte> pixels, IDirect3DTexture9** output);

private:
    bool prepare(D3DPRIMITIVETYPE primitive, IDirect3DBaseTexture9* texture, std::uint32_t count,
                 bool connected);
    HRESULT lock_buffer();
    void draw(std::uint32_t count, std::uint32_t first);
    void vertex(float x, float y, std::uint32_t color, float u = 0.0f, float v = 0.0f);
    void colored(std::span<const ColoredVertex>, D3DPRIMITIVETYPE, bool connected);
    IDirect3DTexture9* font_texture(FontLink&) const noexcept;
    IDirect3DDevice9* device_;
    IDirect3DVertexBuffer9* buffer_ = nullptr;
    RenderVertex* mapped_ = nullptr;
    std::uint32_t capacity_ = 0, count_ = 0, first_ = 0;
    D3DPRIMITIVETYPE primitive_ = D3DPT_TRIANGLESTRIP;
    bool connected_ = false, grow_ = false;
    float alpha_ = 1.0f, depth_ = 0.5f;
    IDirect3DBaseTexture9* texture_ = nullptr;
    IDirect3DBaseTexture9* saved_texture_ = nullptr;
    DWORD saved_fvf_ = 0x142;
    std::vector<TextureRecord> textures_;
    struct FontTexture {
        FontLink* link;
        IDirect3DTexture9* texture;
    };
    std::vector<FontTexture> font_textures_;
    std::atomic_bool rendering_{false};
};

} // namespace skeet::ui
