#pragma once
#include "skeet/esp/projection.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

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
