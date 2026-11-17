#pragma once
#include "skeet/render/backend.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include <optional>

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
