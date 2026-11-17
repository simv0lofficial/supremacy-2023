#pragma once
#include "skeet/rage/ballistics.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/lagcomp/timeline.hpp"
#include "skeet/resolver_runtime/attempts.hpp"
#include "skeet/resolver_runtime/next_attempt.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/script/bridge.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/players/state.hpp"
#include <deque>

namespace skeet::rage {
struct ShotLogSettings;
struct FeedbackEntityBindings;
}
namespace skeet::misc {
class NormalLog;
}

namespace skeet {
// The pending-shot element is the already recovered full 0x1220-byte history
// evaluation. Prefix fields are named through this view without projecting or
// dropping its captured LagRecord, hitbox matrices, or padding.
using ShotFeedbackRecord = lagcomp::HistoryEvaluation;
struct ShotRecordView {
    ShotFeedbackRecord& record;
    std::uint32_t& flags() const noexcept {
        return record.words[0];
    }
    std::int32_t command() const noexcept;
    float time() const noexcept;
    std::int32_t shot_tick() const noexcept;
    std::uint32_t target_handle() const noexcept {
        return record.words[7];
    }
    std::int32_t target_life_id() const noexcept;
    RageVector origin() const noexcept;
    std::int32_t model() const noexcept;
    std::int32_t hitbox_set() const noexcept;
    std::uint32_t fire_weapon() const noexcept {
        return record.words[14];
    }
    std::uint32_t hurt_weapon() const noexcept {
        return record.words[15];
    }
    std::int32_t target() const noexcept;
    std::int32_t target_health() const noexcept;
    std::int32_t record_tick() const noexcept; // shot +144, evaluation prefix
    RageVector target_point() const noexcept;
    std::uint32_t shot_flags() const noexcept {
        return record.state.words[13];
    }
    script::AimShot script_shot() const noexcept;
};
struct ShotFireSnapshot {
    std::uint32_t marker = 0;
    RageVector origin{}, direction{};
};
static_assert(sizeof(ShotFireSnapshot) == 0x1c);
enum class ShotFeedbackNoticeKind {
    miss,
    hit
};
struct ShotFeedbackNotice {
    ShotFeedbackNoticeKind kind{};
    script::AimShot shot{};
    std::uint32_t reason = 0;
    std::int32_t target = 0, hitgroup = 0, damage = 0;
};
struct ShotFeedbackState {
    explicit ShotFeedbackState(script::Bridge& shared_scripts) noexcept : scripts(shared_scripts) {}
    events::GameEventState events;
    script::Bridge& scripts;
    std::vector<ShotFeedbackRecord> pending;
    std::deque<ShotFireSnapshot> fire_snapshots;
    // Native owner+64 count. The inferred-health view numbers its unused
    // slot zero from+5C; actual433ED9DA positions occupy slots1..10,+68..D3.
    // Only count==1/slot1 participates in inferred-health confirmation.
    std::uint32_t recent_position_count = 0;
    std::array<RageVector, 11> recent_positions{};
    // Captured wrapper arguments. Lua dispatch itself runs immediately, before
    // the native flag/counter updates, through the bound scripts member.
    std::vector<ShotFeedbackNotice> notices;
    struct ExtrapolationShot {
        std::int32_t original_command, id, ticks, simulation_tick;
        bool reported = false;
    };
    std::vector<ExtrapolationShot> extrapolation_shots;
};
// Production entity lookups return identity first. Each field is read only
// when the recovered consumer reaches it; health's engine virtual must not be
// introduced by an earlier exists-only test. Decoded fixture values retain
// the same aggregate construction surface.
template <class T> class ShotFeedbackEntityField {
    struct Reader {
        void* object;
        std::ptrdiff_t offset;
        T (*read)(void*, std::ptrdiff_t);
    };
    std::variant<T, Reader> source_;

public:
    ShotFeedbackEntityField(T value = T{}) noexcept : source_(value) {}
    static ShotFeedbackEntityField live(void* object, std::ptrdiff_t offset,
                                        T (*read)(void*, std::ptrdiff_t)) noexcept {
        ShotFeedbackEntityField result;
        result.source_ = Reader{object, offset, read};
        return result;
    }
    operator T() const {
        if (const auto* value = std::get_if<T>(&source_))
            return *value;
        const auto& reader = std::get<Reader>(source_);
        return reader.read(reader.object, reader.offset);
    }
    ShotFeedbackEntityField& operator=(T value) noexcept {
        source_ = value;
        return *this;
    }
};
struct ShotFeedbackEntity {
    bool exists = false;
    ShotFeedbackEntityField<bool> hidden = false;
    ShotFeedbackEntityField<std::int32_t> life_id = -1, health = 0;
    ShotFeedbackEntityField<float> simulation_time = 0; // decoded 0176A381
    ShotFeedbackEntityField<float> last_shot_time = 0; // decoded AE838771
    ShotFeedbackEntityField<float> field_d5fd2e2c = 0;
    ShotFeedbackEntityField<std::uint32_t> weapon_handle = 0xffffffffU;
};
class ShotFeedbackEngine {
public:
    virtual ~ShotFeedbackEngine() = default;
    virtual std::int32_t current_command() = 0; // engine virtual +334
    virtual std::int32_t local_player_index() = 0; // local networkable +28
    virtual float engine_time() = 0; // engine virtual +38, x87 float return
    virtual ShotFeedbackEntity local_player() = 0;
    virtual ShotFeedbackEntity entity_by_index(std::int32_t) = 0;
    virtual ShotFeedbackEntity entity_by_handle(std::uint32_t) = 0;
    // Model-info virtual +4/+80 and studio hitbox-set lookup. Empty is the
    // native missing-model/set/count path, which classifies the ray as spread.
    virtual std::span<const RageLocalHitbox> hitboxes(std::int32_t model, std::int32_t set) = 0;
    // Genuine engine 43468C3C, reached through 4339FA8E for capsules and boxes.
    virtual int clip_hitbox(const RageRay&, const RageLocalHitbox&, const Matrix3x4&,
                            RageTrace&) = 0;
};
struct ShotFeedbackRuntime {
    BallisticEngine& ballistics;
    ShotFeedbackEngine& engine;
    resolver_runtime::AttemptEngineBinding attempt_engine;
    const RageTrigonometry& trigonometry;
    // Publication4341D449 and feedback433866BD/433240DD all call433B8A2B.
    // Resolver slots, counters, timing and history live in this one cache.
    client::PlayerStateCache& players;
    // Native4347BE0E+i*20 is the canonical adjustment's correction_active byte.
    skeet::players::State& player_adjustments;
    const rage::ShotLogSettings& spread_log_settings; //live owner+48, E82BE8DD
    misc::NormalLog& normal_log; //4339DC04, before Lua miss publication
    float now = 0, interval = 0, receive_interval = 0;
    float update_rate = 0, interpolation = 0, interpolation_ratio = 0;
    float correction_minimum = 0, correction_extra = 0;
    std::int32_t maximum_unlag_milliseconds = 0;
    // Native convar read at 43468EA4; a nonzero value suppresses ray tests.
    bool ray_tests_disabled = false;
    const std::uint8_t* resolver_rework = nullptr;
};

float quantize_shot_coordinate(float) noexcept; // complete 433E104F
RageVector quantize_shot_origin(const RageVector&) noexcept; // 43337967
// Complete 43368C91, including x87 receive-time conversion and ring-head time.
bool shot_record_expired(const ShotRecordView&, const client::PlayerState&, ShotFeedbackRuntime&);
// Complete 433866BD. Reason 0 means unknown/resolver; 1 spread; 2 prediction
// error. Only reason zero updates resolver attempts and miss history.
void classify_shot_ray(ShotFeedbackState&, ShotFeedbackRecord&, const RageVector* origin,
                       const RageVector* direction, ShotFeedbackRuntime&);
// Complete 433240DD: expiry, captured timing, game-event grouping/matching,
// ray reconstruction, hit/miss updates, death/life changes and inferred hits.
void reconcile_shot_feedback(ShotFeedbackState&, ShotFeedbackRuntime&);
// Required-live production forms used by the canonical BA8 owner. The
// two-argument/value-projection forms above remain explicit fixture APIs.
bool shot_record_expired(const ShotRecordView&, const client::PlayerState&, ShotFeedbackRuntime&,
                         const rage::FeedbackEntityBindings&);
void classify_shot_ray(ShotFeedbackState&, ShotFeedbackRecord&, const RageVector* origin,
                       const RageVector* direction, ShotFeedbackRuntime&,
                       const rage::FeedbackEntityBindings&);
void reconcile_shot_feedback(ShotFeedbackState&, ShotFeedbackRuntime&,
                             const rage::FeedbackEntityBindings&);
} // namespace skeet
