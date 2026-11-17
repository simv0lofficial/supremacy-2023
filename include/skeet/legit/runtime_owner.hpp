#pragma once
#include "owner.hpp"
#include "command_source.hpp"
#include <memory>

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class CrtRuntimeOwner;
class NativeStartupSources;
}
namespace skeet::legit {
// Native phase4 automatic workspace. The explicit-input constructors retain
// a differential-fixture seam; production needs no persistent seed arrays.
struct CommandFrameInputs {
    lagcomp::RecordInterpolationScratch record_interpolation;
    AccuracyMatrixScratch accuracy_interpolation;
    std::array<Matrix3x4, 256> attachment_matrices;
    std::array<Matrix3x4, 256> saved_matrices;
};
// Genuine RNG/math/engine boundary services. The explicit constructor supports
// authored standalone fixtures using the same complete recovered source body.
struct RuntimeServices {
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    lagcomp::RotationMath rotation;
    float (*exponential)(float);
    TriggerTraceBindings trace;
};

// Owns the single source settings/runtime state and the real4347FEBC factory
// context. No initial player/Command/target is retained. All phase4 physical
// contexts are formed from the actual arguments and canonical engine cells.
// Construction registers nothing and calls no engine/game function.
class RuntimeOwner final {
public:
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, client::CrtRuntimeOwner&,
                 esp::EspRuntimeOwner&, CommandFrameInputs&);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, CommandFrameInputs&, RuntimeServices);
    RuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                 engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                 client::CommandCaptureRuntimeOwner&, RuntimeServices);
    ~RuntimeOwner();
    RuntimeOwner(const RuntimeOwner&) = delete;
    RuntimeOwner& operator=(const RuntimeOwner&) = delete;
    RuntimeOwner(RuntimeOwner&&) = delete;
    RuntimeOwner& operator=(RuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    Settings& settings() noexcept;
    RuntimeState& state() noexcept;
    client::LegitCommandOwner& commands() noexcept;
    OwnerBindings& factory_bindings() noexcept;
    // Binding selects the existing real allocation/constructor body only.
    // NativeStartupSourceGraph may instead consume factory_bindings() in its
    // eight-owner bundle; do not bind the same initializer through both paths.
    void bind_initializer(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::legit
