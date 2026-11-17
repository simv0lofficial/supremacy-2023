#pragma once

// Rendering Subsystem Declarations
#include <array>
#include <cstdint>
#include <string_view>
#include "skeet/render_queue.hpp"
#include <d3d9.h>
#include <span>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include "skeet/esp/projection.hpp"
#include <cstddef>
#include <variant>
#include "skeet/font_cache.hpp"
#include "skeet/render/backend.hpp"
#include <optional>
#include "skeet/engine/native_data_image.hpp"
#include "includes.h"


// Declarations from font_cache.hpp

namespace skeet::ui {
class FontRuntime;
struct GlyphMetrics {
    std::int32_t advance_first = 0;
    std::int32_t advance_second = 0;
};
struct GlyphCache {
    void* context;
    bool (*prepare)(void*, char16_t);
    std::int32_t (*height)(void*) = nullptr;
    bool (*metrics)(void*, char16_t, GlyphMetrics&) = nullptr;
    std::int32_t (*internal_leading)(void*) = nullptr;
    std::int32_t (*ascent)(void*) = nullptr;
    FontRuntime* runtime = nullptr;
};
struct FontLink {
    GlyphCache* cache = nullptr;
    FontLink* next = nullptr;
};
using FontSet = std::array<FontLink, 7>;
struct FontBackend {
    float scale = 1.0f;
    FontSet default_fonts;
    FontSet scaled_fonts;
};
FontSet& select_fonts(FontBackend& backend, std::uint32_t flags) noexcept;
void prepare_glyphs(FontBackend* backend, std::u16string_view text, std::uint32_t flags);
} // namespace skeet::ui

// Declarations from font_metrics.hpp

namespace skeet::ui {
std::int32_t font_height(FontBackend& backend, std::uint32_t index);
}

// Declarations from backend.hpp

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

// Declarations from backend_container.hpp

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

// Declarations from beam_geometry.hpp

namespace skeet::render {

// Native segment at owner +0x200. The fourth position lane is copied by
// NextSeg but does not participate in its three-dimensional arithmetic.
struct BeamSegment {
    esp::Vector3 position{};
    float position_w = 0.0f;
    std::uint32_t rgba = 0xFFFFFFFFu; // memory order R,G,B,A
    float texcoord = 0.0f;
    float width = 0.0f;
};
static_assert(sizeof(BeamSegment) == 28);
static_assert(offsetof(BeamSegment, rgba) == 16);
static_assert(offsetof(BeamSegment, width) == 24);

struct BeamVertex {
    esp::Vector3 position;
    std::uint32_t bgra;
    std::array<float, 2> uv0, uv1;
    esp::Vector3 tangent_s, tangent_t;
};
static_assert(sizeof(BeamVertex) == 56);

struct BeamGeometry {
    std::vector<BeamVertex> vertices;
    std::vector<std::uint16_t> indices;
    bool indexed = false;
};

// Complete 43320D77. One segment remains pending until the next segment
// supplies its width axis. The last call also emits the pending endpoint.
// Indexed mode appends triangles to a shared external geometry; strip mode
// preserves the native owned MeshBuilder's paired-vertex order.
class BeamSegmentBuilder {
public:
    BeamSegmentBuilder(BeamGeometry& output, std::int32_t segment_count, esp::Vector3 camera,
                       bool indexed = false) noexcept;
    void next_segment(const BeamSegment& segment);
    std::int32_t segments_seen() const noexcept {
        return segments_seen_;
    }

private:
    void emit_pair(esp::Vector3 width_axis, esp::Vector3 tangent);
    void append_quad();
    BeamGeometry& output_;
    esp::Vector3 camera_, previous_width_axis_{};
    BeamSegment pending_{};
    std::int32_t segment_count_, segments_seen_ = 0;
    std::uint16_t first_vertex_;
    bool indexed_;
};

// Replaces output with one complete beam. A builder may instead append
// multiple indexed beams to the same BeamGeometry.
void build_beam_geometry(std::span<const BeamSegment> segments, esp::Vector3 camera,
                         BeamGeometry& output, bool indexed = false);

// The captured x86 IMesh::LockMesh descriptor. The renderer writes only the
// position, color, UV0/UV1 and tangent S/T channels used by native NextSeg.
struct BeamMeshDescriptor {
    std::array<std::int32_t, 21> vertex_layout{};
    std::array<void*, 18> vertex_data{};
    std::int32_t first_vertex = 0;
    std::uint32_t vertex_offset = 0;
    std::uint16_t* indices = nullptr;
    std::uint32_t index_offset = 0, first_index = 0;
    std::int32_t index_size = 0;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(BeamMeshDescriptor) == 0xB4);
static_assert(offsetof(BeamMeshDescriptor, vertex_data) == 0x54);
static_assert(offsetof(BeamMeshDescriptor, first_vertex) == 0x9C);
static_assert(offsetof(BeamMeshDescriptor, indices) == 0xA4);
#endif

// Uses the already acquired native dynamic IMesh. Material/context binding
// belongs to the caller. This performs the real mesh lock, channel scatter,
// index generation, unlock and draw; no internal geometry callback is used.
void draw_beam_geometry(void* mesh, const BeamGeometry& geometry);

} // namespace skeet::render

// Declarations from device_listener.hpp

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

// Declarations from dpi_bridge.hpp

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

// Declarations from font_runtime.hpp

namespace skeet::ui {
struct GlyphImage {
    float left = 0, top = 0, right = 0, bottom = 0;
    float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
    std::int32_t advance_first = 0, advance_second = 0;
};
struct GlyphAtlas {
    std::int32_t width = 0, height = 0;
    std::span<const std::uint32_t> pixels;
};

// GDI font, Unicode coverage, glyph records and the growing ARGB atlas.
// Flags: 1 bold, 2 ClearType/ABC widths, 4 one-pixel outline.
class FontRuntime {
public:
    FontRuntime(std::u16string_view face, std::int32_t height, std::uint32_t flags);
    ~FontRuntime();
    FontRuntime(const FontRuntime&) = delete;
    FontRuntime& operator=(const FontRuntime&) = delete;
    bool valid() const noexcept;
    bool supports(char16_t character) const noexcept;
    bool prepare(char16_t character);
    void prepare(std::u16string_view text);
    bool glyph(char16_t character, GlyphImage& out) const noexcept;
    GlyphAtlas atlas() const noexcept;
    void mark_dirty() noexcept;
    bool consume_dirty() noexcept;
    GlyphCache& cache() noexcept;
    std::int32_t height() const noexcept;
    std::int32_t ascent() const noexcept;
    std::int32_t internal_leading() const noexcept;
    std::int32_t external_leading() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    GlyphCache cache_{};
};

std::int32_t font_internal_leading(FontBackend& backend, std::uint32_t index);
std::int32_t font_ascent(FontBackend& backend, std::uint32_t index);
} // namespace skeet::ui

// Declarations from frame_runtime.hpp

namespace skeet::ui {
// Native source global 43468350 is a pointer to the external render bridge,
// whose parameterless frame methods in the working capture are slots 1,10,11,2.
// The supplied earlier image used slots 9/10 for the middle pair. This is not the D3D9
// device vtable. The shutdown word is the native 4347C9FC lifecycle gate.
class FrameRuntime {
public:
    FrameRuntime(RendererContainer& renderer, void* const* render_bridge,
                 const volatile std::uint32_t& shutdown) noexcept
        : renderer_(renderer), bridge_(render_bridge), shutdown_(shutdown) {}
    void render(); // 43384182
private:
    RendererContainer& renderer_;
    void* const* bridge_;
    const volatile std::uint32_t& shutdown_;
};
} // namespace skeet::ui

// Declarations from icon_cache.hpp

namespace skeet::esp {
struct WeaponIconProvider;
}
namespace skeet::ui {
struct RageSelectorIconProvider;
enum class ImageFormat : std::int32_t {
    svg = 0,
    png = 1,
    jpeg = 2
};

// Native CUtlBuffer prefix passed to the game's image codecs. All allocations
// come from its IMemAlloc, including allocations made by the overflow callback.
struct EngineImageBuffer {
    std::byte* memory = nullptr;
    std::int32_t capacity = 0, grow_size = 0, get = 0, put = 0;
    std::uint8_t error = 0, flags = 0;
    std::uint16_t reserved = 0;
    std::int32_t tab = 0, maximum_put = 0, offset = 0;
#if defined(_M_IX86) || defined(__i386__)
    using Overflow = bool(__thiscall*)(EngineImageBuffer*, std::int32_t);
#else
    using Overflow = bool (*)(EngineImageBuffer*, std::int32_t);
#endif
    Overflow get_overflow = nullptr, put_overflow = nullptr;
    std::uint32_t trailing_flags = 0;
    void* allocator = nullptr;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(EngineImageBuffer, error) == 0x14);
static_assert(offsetof(EngineImageBuffer, maximum_put) == 0x1c);
static_assert(offsetof(EngineImageBuffer, put_overflow) == 0x28);
static_assert(offsetof(EngineImageBuffer, allocator) == 0x30);
#endif

// These are actual engine addresses: enqueue/execute are the thread-pool entry
// points, svg/png/jpeg are the image-library functions at native globals
// 4346D68C/4346D688/4346D684. The material/render-context chain supplies its pool.
struct ImageDecoderBindings {
    void* render_context_source = nullptr; // 43468E98; slot11 -> slot29 -> +88
    void* allocator = nullptr; // IMemAlloc, native43467204
    std::uintptr_t enqueue = 0, execute = 0;
    std::uintptr_t svg = 0, png = 0, jpeg = 0;
};
class ImageRasterizer {
public:
    explicit ImageRasterizer(ImageDecoderBindings bindings) : bindings_(bindings) {}
    bool decode(ImageFormat format, std::span<const std::byte> bytes, std::int32_t& width,
                std::int32_t& height, std::vector<std::uint32_t>& rgba);

private:
    ImageDecoderBindings bindings_;
};

struct IconBindings {
#if defined(_M_IX86) || defined(__i386__)
    using ItemSystemFactory = void*(__cdecl*)();
#else
    using ItemSystemFactory = void* (*)();
#endif
    void* filesystem; // IBaseFileSystem, native43467F68 object's +4; slots0/2/3/7
    void*& item_system; // canonical43476424, also published by post-initialization
    ItemSystemFactory const& item_system_factory; //live43468B9C
    ImageDecoderBindings images;
    IconBindings(void* files, void*& items, ItemSystemFactory const& factory,
                 ImageDecoderBindings decoder)
        : filesystem(files), item_system(items), item_system_factory(factory), images(decoder) {}
};
struct CachedIcon {
    std::int32_t texture = -1, width = 0, height = 0;
};
class IconCache {
public:
    // Early CRT owns the65 native IDs before engine/D3D discovery exists.
    IconCache() noexcept = default;
    IconCache(RendererBackend& renderer, IconBindings bindings);
    void bind_runtime(RendererBackend&, IconBindings);
    bool runtime_bound() const noexcept {
        return runtime_.has_value();
    }
    void initialize_native_cache() noexcept; //433689B6, reset the same65 cached texture IDs
    bool path(std::uint16_t definition, std::string& output);
    bool load(std::uint16_t definition, std::int32_t requested_width, std::int32_t requested_height,
              CachedIcon& output, bool dimensions = true);
    std::int32_t cached_texture(std::uint16_t definition) const noexcept;
    std::int32_t load_svg(const std::string& path, std::int32_t height, std::int32_t existing = -1);
    esp::WeaponIconProvider weapon_provider() noexcept;
    RageSelectorIconProvider selector_provider() noexcept;
    ImageRasterizer& image_rasterizer();

private:
    struct Runtime {
        RendererBackend& renderer;
        IconBindings bindings;
        ImageRasterizer rasterizer;
        Runtime(RendererBackend& target, IconBindings acquired)
            : renderer(target), bindings(acquired), rasterizer(acquired.images) {}
    };
    Runtime& runtime();
    std::vector<std::byte> read_game_file(const std::string& path);
    std::optional<Runtime> runtime_;
    std::array<std::int32_t, 65> textures_{};
};
} // namespace skeet::ui

// Declarations from menu_background.hpp

namespace skeet::ui {
class MenuWindow;
class RendererBackend;

inline constexpr std::uint32_t menu_background_key = 0xd60978b0;

// Renderer context constructor,4348F8DA..4348F99B. The backend retains the
// decoded pixel bytes and owns their normal device upload/recreation.
std::uint32_t create_menu_background(RendererBackend& renderer);

// Main-window initializer43485A35..43485A49, after the tab-atlas load.
void assign_menu_background(MenuWindow& window, const RendererBackend& renderer) noexcept;
}

// Declarations from menu_window_assets.hpp

namespace skeet::ui {
class MenuWindow;
class RendererBackend;

struct MainMenuAssets {
    std::uint32_t tab_atlas = UINT32_MAX; // native mainwindow+D8
};

//434801C9's resource25D9 is the provided decoded384x48 RGBA atlas.
std::uint32_t create_menu_tab_atlas(RendererBackend& renderer);

// Complete mainwindow vslot+4,43485A05. Composition must already own its tabs.
void initialize_main_menu_assets(MenuWindow& window, RendererBackend& renderer,
                                 MainMenuAssets& assets);
}

// Declarations from renderer_assets.hpp

namespace skeet::client {
class ClientRuntimeStorage;
}
namespace skeet::engine {
class NativeDataImage;
struct LoadedModuleEnvironment;
struct NativeImageEnvironment;
}
namespace skeet::ui {
class StartupRenderer;
class RendererContainer;
class RendererDeviceListener;
// Bind the actual loaded build's codec/job functions and allocator export,
// then borrow the canonical filesystem/ItemSystem slots used by initialization.
IconBindings initialize_build_icon_bindings(engine::NativeDataImage&);
IconBindings initialize_build_icon_bindings(engine::NativeDataImage&,
                                            const engine::LoadedModuleEnvironment&,
                                            engine::NativeImageEnvironment);

class RendererAssets {
public:
    explicit RendererAssets(client::ClientRuntimeStorage&);
    RendererAssets(client::ClientRuntimeStorage&, StartupRenderer&);
    // Explicit already-acquired engine boundaries for independent source
    // composition. Every pointer/reference must match its canonical cell;
    // the ordinary overload performs the genuine loaded-module discovery.
    RendererAssets(client::ClientRuntimeStorage&, StartupRenderer&, IconBindings);
    void bind_renderer(StartupRenderer&);
    void bind_renderer(StartupRenderer&, IconBindings);
    bool renderer_bound() const noexcept;
    ~RendererAssets();
    RendererAssets(const RendererAssets&) = delete;
    RendererAssets& operator=(const RendererAssets&) = delete;
    RendererAssets(RendererAssets&&) = delete;
    RendererAssets& operator=(RendererAssets&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    StartupRenderer& startup_renderer() const;
    RendererContainer& renderer() const;
    RendererDeviceListener& device_listener() const noexcept;
    IconCache& icon_cache() noexcept;
    StartupIcons& startup_icons() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}

// Declarations from startup_icons.hpp

namespace skeet::ui {
class IconCache;

struct StartupIcons {
    std::int32_t& grenade; //43468E3C
    std::int32_t& inferno; //4346842C
    std::int32_t& bomb; //43467DA8
};

// Complete433283A5. Uses the cache belonging to renderer4347690C.
void initialize_startup_icons(IconCache& cache, StartupIcons& icons);
}

// Declarations from startup_renderer.hpp

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

// Declarations from render_queue.hpp

namespace skeet::ui {
class RendererContainer;

struct RectangleCommand {
    std::int32_t x, y, width, height;
    std::uint32_t color;
};
struct GradientCommand {
    std::int32_t x, y, width, height;
    std::uint32_t first_color, second_color;
};
struct FourColorGradientCommand {
    std::int32_t x, y, width, height;
    // Native vertex order; corner orientation is not yet recovered.
    std::array<std::uint32_t, 4> colors;
};
struct LinePoint {
    std::int32_t x, y;
};
struct LineCommand {
    std::int32_t from_x, from_y, to_x, to_y;
    std::uint32_t color;
};
struct TriangleCommand {
    std::int32_t first_x, first_y, second_x, second_y, third_x, third_y;
    std::uint32_t color;
};
struct CircleCommand {
    std::int32_t x, y;
    std::uint32_t color;
    float radius, rotation, fraction;
};
struct CircleOutlineCommand {
    std::int32_t x, y;
    std::uint32_t color;
    float radius, rotation, fraction, thickness;
};

struct TextureCommand {
    std::int32_t x, y;
    std::uint32_t texture;
    std::uint32_t color;
    std::int32_t source_x, source_y, source_width, source_height;
    std::uint32_t mode;
    float scale;
};
static_assert(sizeof(TextureCommand) == 40);

std::uint32_t desaturate_color(std::uint32_t color, bool enabled) noexcept;
std::uint32_t subtract_color(std::uint32_t color, std::uint32_t amount) noexcept;
std::uint32_t add_color(std::uint32_t color, std::uint32_t amount) noexcept;

struct RenderQueue {
    std::vector<std::byte> commands;
    bool desaturate = false;
    bool bounds_reset = false;
    std::array<std::int32_t, 4> bounds{};
    float depth = 0.5f;
    float alpha = 1.0f;
    FontBackend* fonts = nullptr;
    Point screen() const; //433B0530 -> 433F49E6, supplied queue's renderer

    // Header: low 27 bits are the absolute end offset, high 5 bits the opcode.
    std::byte* append(std::uint32_t opcode, std::uint32_t payload_size);
    void rectangle(RectangleCommand command);
    void rectangle_outline(RectangleCommand command, std::int32_t thickness);
    void outlined_rectangle(RectangleCommand command, std::uint32_t border_color,
                            std::int32_t thickness);
    void four_color_gradient(FourColorGradientCommand command);
    void vertical_gradient(GradientCommand command);
    void horizontal_gradient(GradientCommand command);
    void line(LineCommand command);
    void polyline(std::span<const LinePoint> points, std::uint32_t color);
    void triangle(TriangleCommand command);
    void circle(CircleCommand command);
    void circle_outline(CircleOutlineCommand command);
    void texture(TextureCommand command);
    void text(std::int32_t x, std::int32_t y, std::uint32_t color, std::uint32_t flags,
              std::int32_t maximum_width, std::u16string_view content);
    void set_bounds(const std::array<std::int32_t, 4>& value);
    std::array<std::int32_t, 4> get_bounds() const noexcept;
    void reset_bounds(bool reset);
    bool is_bounds_reset() const noexcept;
    void set_alpha(float value);
    float get_alpha() const noexcept;
    void set_depth(float value);
    float get_depth() const noexcept;
    void set_desaturation(bool enabled) noexcept;

private:
    friend class RendererContainer;
    // Borrowed from the container that owns this frame. This never follows
    // the current global renderer after a callback changes that global slot.
    RendererContainer* renderer_ = nullptr;
};

} // namespace skeet::ui

// Declarations from text_metrics.hpp

namespace skeet::ui {
Point measure_text(RenderQueue& queue, std::u16string_view content, std::uint32_t flags);
}
