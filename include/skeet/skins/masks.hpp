#pragma once
#include "skeet/skins/gloves.hpp"
#include <array>
#include <string_view>

namespace skeet::skins {
struct MaskEntity : GloveEntity {
    // Native field 0x43467E8C points to the player's model-header wrapper.
    // The native gate requires both this pointer and its first word nonzero.
    const std::uint32_t* model_header = nullptr;
    std::uint32_t parent_handle = invalid_glove_handle; // field 0x43469344 - 0x18
    std::array<float, 3> local_origin{}; // field 0x43468D30
    std::array<float, 3> local_angles{}; // field 0x43468368
    std::uint16_t bits_43467044 = 0;
    bool flag_43468bd4 = false, flag_43467964 = false;
    std::uint32_t flags_43469224 = 0;
};
struct MaskOwnerState {
    std::uint32_t handle = invalid_glove_handle; // owner +0x54
    // The native constructor resolves class hash 0xF89B2C53. Its factory must
    // construct a MaskEntity adapter through the same real 0x43364942 chain.
    GloveFactory* factory = nullptr; // owner +0x58
};
class MaskEngine : public GloveEngine {
public:
    virtual MaskEntity* resolve(std::uint32_t handle) override = 0;
    virtual bool parent_function_available() = 0; // resolved 0x4346D644
    virtual const void* render_model(MaskEntity&) = 0; // renderable +0x20
    virtual std::int32_t attachment(MaskEntity&, std::string_view name) = 0; // renderable +0x84
    virtual void set_parent(MaskEntity&, MaskEntity& parent,
                            std::int32_t attachment) = 0; // 0x4346D644
    // Call the optional resolved 0x43468C10 when present. The field writes
    // remain in the source; this method only performs that engine operation.
    virtual void invalidate_transform(MaskEntity&, std::int32_t flags) = 0;
    // Actual decoded virtual call +0x3B8. Its precise engine name is not
    // established, so no Source SDK method identity is assigned to it.
    virtual void entity_boolean_3b8(MaskEntity&, bool) = 0;
};
// Exact constructor 0x4347EB9D catalog, with all 22 native selection values.
std::span<const std::string_view, 22> mask_model_paths() noexcept;
// Complete native 0x43408846. Factory creation is the same engine sequence as
// gloves; model lookup/loading uses the recovered 0x43331B0C implementation.
void apply_mask(const Settings&, MaskOwnerState&, MaskEntity* local, MaskEngine&, ModelInfo&,
                StringTables&);
} // namespace skeet::skins
