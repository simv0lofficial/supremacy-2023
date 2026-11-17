#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace skeet::effects {
struct BlurRectangle {
    std::int16_t x, y, width, height;
};
static_assert(sizeof(BlurRectangle) == 8);
struct BlurBindings {
    void* engine = nullptr; // 4346A514
    void* material_system = nullptr; // 4346A4E0
};
// IMesh::LockMesh output. Layout/data entries 0,3,6,14,15 are position,
// normal, first UV channel, tangent S and tangent T. The other engine fields
// pass back unchanged to UnlockMesh. Native x86 MeshDesc_t is 0xB4 bytes.
struct BlurMeshDescriptor {
    std::array<std::int32_t, 21> vertex_layout{};
    std::array<void*, 18> vertex_data{};
    std::int32_t first_vertex = 0;
    std::uint32_t vertex_offset = 0;
    std::uint16_t* indices = nullptr;
    std::uint32_t index_offset = 0, first_index = 0;
    std::int32_t index_size = 0;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(BlurMeshDescriptor) == 0xB4);
static_assert(offsetof(BlurMeshDescriptor, vertex_data) == 0x54);
static_assert(offsetof(BlurMeshDescriptor, first_vertex) == 0x9C);
static_assert(offsetof(BlurMeshDescriptor, indices) == 0xA4);
#endif

class BlurEffect {
public:
    explicit BlurEffect(BlurBindings bindings = {}) : bindings(bindings) {}
    BlurBindings bindings;
    // Material fields +0C,+14,+18,+10, then texture fields +20,+24,+1C.
    std::array<void*, 4> materials{};
    std::array<void*, 3> textures{};
    std::array<std::uint32_t, 3> material_hashes{0x2EA680DFu, 0x3141F4D7u, 0x3641F748u};
    std::string texture_group = "RenderTargets";
    std::array<std::string, 3> texture_names{"_rt_SmallFB0", "_rt_SmallFB1", "_rt_FullFrameFB"};

    // Each input keeps its low 16 bits. Full batches ignore further entries.
    void draw(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height); // 433F2AE2
    std::span<const BlurRectangle> pending() const noexcept {
        return rectangles_;
    }
    bool initialize_resources(); // 433C0291
    void release_resources(); // 433E2BC0
    void render(); // 433C8896, including clearing failed batches
private:
    void apply(); // 433B1183
    std::vector<BlurRectangle> rectangles_;
};
} // namespace skeet::effects
