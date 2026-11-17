#pragma once
#include "entity_observer_receiver.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace skeet::esp {
class SoundHistoryReceiver;
}
namespace skeet::lagcomp {
struct HistoryRecordingReceiver;
}
namespace skeet::misc {
class ClanTagRuntime;
class GrenadeCommandRuntime;
}
namespace skeet::prediction {
struct EngineEntityUpdateOwner;
struct ViewmodelPredictionObserver;
}
namespace skeet::rage {
struct FeedbackEntityOwner;
struct EntityUpdateBindings;
}
namespace skeet::script {
class ScriptEntityObserver;
}
namespace skeet::skins {
class NativeEntityCosmetics;
class NativeInventoryRuntime;
class NativeSequenceRuntime;
struct SequenceEntityObserverReceiver;
}
namespace skeet::effects {
struct EffectsCommandOwner;
}
namespace skeet::client {
struct PredictionHistoryOwner;
struct EntityCommandOwner;
struct SharedEspCommandOwner;
class TargetMaskOwner;
//Registrar4333BF32, vector43475BA8/BAC. This is separate from the command,
//empty-tick, recharge and engine-prediction registries.
enum class EntityObserverId : std::uint32_t {
    prediction_history = 0x43471DD4u,
    clan_tag = 0x4346EEA0u,
    history_recording = 0x43471D6Cu,
    mask = 0x43470560u,
    shot_feedback = 0x4346F68Cu,
    entity_commands = 0x4346FBF0u,
    scripts = 0x434729C0u,
    rage = 0x43472480u,
    target_mask = 0x4346F98Cu,
    pickup_history = 0x43472670u,
    prediction_update = 0x43472388u,
    shared_esp = 0x4346F2D0u,
    grenades = 0x4346DC78u,
    viewmodel_lag = 0x43472644u,
    sequences = 0x4346DC68u,
    effects = 0x4346FAFCu,
    inventory = 0x4347030Cu,
    gloves = 0x4346FEBCu,
};
struct EntityObserverTable {
    EntityObserverId id;
    std::array<std::uint32_t, 4> methods;
};
std::span<const EntityObserverTable> native_entity_observer_tables() noexcept;
std::optional<EntityObserverId> identify_entity_observer(std::uint32_t) noexcept;
bool entity_observer_source_bound(EntityObserverId, std::uint32_t slot);

struct EntityObserverRegistry {
    EntityObserverRegistry(engine::PreservedInitialization&, engine::JobMemory);
    EntityObserverRegistry(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    EntityObserverRegistry(const EntityObserverRegistry&) = delete;
    EntityObserverRegistry& operator=(const EntityObserverRegistry&) = delete;
    EntityObserverRegistry(EntityObserverRegistry&&) = delete;
    EntityObserverRegistry& operator=(EntityObserverRegistry&&) = delete;
    // Actual constructors supply both their physical endpoint and their same
    // canonical source view. Publication follows at each recorded table write.
    void append_prediction_history(NativeEntityObserverReceiver&, PredictionHistoryOwner&);
    void append_clan_tag(NativeEntityObserverReceiver&, misc::ClanTagRuntime&);
    void append_mask(NativeEntityObserverReceiver&, skins::NativeEntityCosmetics&);
    void append_feedback(NativeEntityObserverReceiver&, rage::FeedbackEntityOwner&);
    void append_entity_commands(NativeEntityObserverReceiver&, EntityCommandOwner&);
    void append_scripts(NativeEntityObserverReceiver&, script::ScriptEntityObserver&);
    void append_rage(NativeEntityObserverReceiver&, const rage::EntityUpdateBindings&);
    void append_target_mask(NativeEntityObserverReceiver&, TargetMaskOwner&);
    void append_prediction_update(NativeEntityObserverReceiver&,
                                  prediction::EngineEntityUpdateOwner&);
    void append_shared_esp(NativeEntityObserverReceiver&, SharedEspCommandOwner&);
    void append_grenades(NativeEntityObserverReceiver&, misc::GrenadeCommandRuntime*);
    void append_viewmodel_lag(NativeEntityObserverReceiver&,
                              prediction::ViewmodelPredictionObserver&);
    void append_effects(NativeEntityObserverReceiver&, effects::EffectsCommandOwner&);
    void append_inventory(NativeEntityObserverReceiver&, skins::NativeInventoryRuntime&);
    void append_gloves(NativeEntityObserverReceiver&, skins::NativeEntityCosmetics&);
    void append(lagcomp::HistoryRecordingReceiver&);
    void append(esp::SoundHistoryReceiver&);
    void append(skins::SequenceEntityObserverReceiver&, skins::NativeSequenceRuntime&);
    void publish_prediction_history(NativeEntityObserverReceiver&) const noexcept;
    void publish_clan_tag(NativeEntityObserverReceiver&) const noexcept;
    void publish_mask(NativeEntityObserverReceiver&) const noexcept;
    void publish_feedback(NativeEntityObserverReceiver&) const noexcept;
    void publish_entity_commands(NativeEntityObserverReceiver&) const noexcept;
    void publish_scripts(NativeEntityObserverReceiver&) const noexcept;
    void publish_rage(NativeEntityObserverReceiver&) const noexcept;
    void publish_target_mask(NativeEntityObserverReceiver&) const noexcept;
    void publish_prediction_update(NativeEntityObserverReceiver&) const noexcept;
    void publish_shared_esp(NativeEntityObserverReceiver&) const noexcept;
    void publish_grenades(NativeEntityObserverReceiver&) const noexcept;
    void publish_viewmodel_lag(NativeEntityObserverReceiver&) const noexcept;
    void publish_effects(NativeEntityObserverReceiver&) const noexcept;
    void publish_inventory(NativeEntityObserverReceiver&) const noexcept;
    void publish_gloves(NativeEntityObserverReceiver&) const noexcept;
    void publish(lagcomp::HistoryRecordingReceiver&) const noexcept;
    void publish(esp::SoundHistoryReceiver&) const noexcept;
    void publish(skins::SequenceEntityObserverReceiver&) const noexcept;
    std::span<void* const> owners() const noexcept {
        return owners_.entries();
    }
    engine::OwnerRegistry& native_storage() const noexcept {
        return owners_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    //433E4C98 alone releases the current native buffer, without resetting its
    //three BSS cells. Ordinary C++ destruction releases no buffer or receiver.
    void finalize_storage() const;
    void require_sources(std::uint32_t slot) const;
    void update_status(const void* status) const; //slot0
    void update_entities(void* player, std::uintptr_t incoming) const; //slot1
    void reset_update_state(std::int32_t selected_tick) const; //slot2, EDX preserved
    void incoming_complete(void* saved_player, std::int32_t acknowledged_delta) const; //slot3
private:
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::OwnerRegistry& owners_;
    engine::JobMemory memory_;
    void append_receiver(NativeEntityObserverReceiver&, void* source);
    void append_native(NativeEntityObserver&);
};
} //namespace skeet::client
