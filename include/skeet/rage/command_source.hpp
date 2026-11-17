#pragma once
#include "command_owner.hpp"
#include "skeet/esp/hitbox_highlight.hpp"
#include <variant>

namespace skeet::rage {
class PredictionMaskOwner;
// Production endpoints form their physical contexts only at invocation. The
// registered endpoint retains source ownership, never a player or frame view.
class CommandSource {
public:
    virtual ~CommandSource() = default;
    virtual PreambleState& state() noexcept = 0;
    virtual AcquisitionStorage& acquisition() noexcept = 0;
    virtual std::int32_t& shot_index() noexcept = 0;
    virtual esp::HitboxHighlightState& action_state() noexcept = 0;
    virtual std::array<animation::AnimationAngleSample, 2>& pending_angles() noexcept = 0;
    virtual PredictionMaskOwner* const& prediction_masks() const noexcept = 0;
    virtual void require_factory(const AimbotSettings&, const GlobalSettings&,
                                 const esp::AimbotVisualizationConfig&,
                                 const esp::HitboxHighlightState&) const = 0;
    virtual void phase2(void*, void*, client::Command&, std::array<float, 3>&, std::uint8_t&) = 0;
    virtual void process(client::CommandFrame&, void*, client::Command&, std::uint8_t&) = 0;
    virtual void phase6(const client::CommandFrame&, const client::Command&) = 0;
    virtual void capture(const client::CommandFrame&) noexcept = 0;
};
class CommandEndpoint {
    std::variant<CommandOwner*, CommandSource*> source_;

public:
    CommandEndpoint(CommandOwner& value) noexcept : source_(&value) {}
    CommandEndpoint(CommandSource& value) noexcept : source_(&value) {}
    PreambleState& state() const noexcept;
    AcquisitionStorage& acquisition() const noexcept;
    std::int32_t& shot_index() const noexcept;
    esp::HitboxHighlightState& action_state() const noexcept;
    std::array<animation::AnimationAngleSample, 2>& pending_angles() const noexcept;
    PredictionMaskOwner* const& prediction_masks() const noexcept;
    void require_factory(const AimbotSettings&, const GlobalSettings&,
                         const esp::AimbotVisualizationConfig&,
                         const esp::HitboxHighlightState&) const;
    void phase2(void*, void*, client::Command&, std::array<float, 3>&, std::uint8_t&);
    void process(client::CommandFrame&, void*, client::Command&, std::uint8_t&);
    void phase6(const client::CommandFrame&, const client::Command&);
    void capture(const client::CommandFrame&) noexcept;
};
} //namespace skeet::rage
