#pragma once
#include "skeet/skins/config.hpp"
#include "skeet/skins/model_resources.hpp"
#include <optional>
#include <span>

namespace skeet::skins {
struct AgentModelState {
    std::int32_t original_model = -1;
    std::uint32_t original_handle = 0;
};
struct AgentEntity {
    std::uint32_t handle = 0, ragdoll_handle = 0xFFFFFFFF;
    std::int32_t team = 0;
    bool dormant = false, dead = false;
    std::int16_t model_index = -1;
};
class AgentEngine {
public:
    virtual ~AgentEngine() = default;
    virtual bool demo_playback() = 0;
    virtual bool hltv() = 0;
    virtual std::span<const std::string_view> agent_paths(std::int32_t team) = 0;
    // Native engine helper 0x4346D64C returns the two associated models at +8/+C.
    virtual std::optional<std::array<std::string_view, 2>>
    support_models(std::string_view path) = 0;
    virtual AgentEntity* ragdoll(std::uint32_t handle) = 0;
};
// Complete 0x433466F7: live entity/ragdoll selection and original-model restore.
void apply_agent_model(const Settings&, const AgentModelState&, AgentEntity*, AgentEngine&,
                       ModelInfo&, StringTables&);
} // namespace skeet::skins
