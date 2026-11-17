#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace skeet::events {

// 0x43359C70: FNV-1a followed by the build's final XOR. Null is accepted.
std::uint32_t event_hash(const char* text) noexcept;

inline constexpr std::uint32_t weapon_fire_hash = 0xEA688D0F;
inline constexpr std::uint32_t bullet_impact_hash = 0x19459119;
inline constexpr std::uint32_t player_hurt_hash = 0x89FA8001;
inline constexpr std::uint32_t player_death_hash = 0x3461B584;

enum class FieldType : std::uint32_t {
    floating = 1,
    string = 2,
    integer = 5,
    boolean = 6
};
union FieldValue {
    std::uint32_t integer;
    float floating;
    const char* string;
    bool boolean;
    constexpr FieldValue() : string(nullptr) {}
};

// The listener's typed field visitor, native vtable 4346F114. Failed matches
// leave the previous value untouched; false stops the engine's key scan.
struct FieldQuery {
    std::uint32_t name_hash;
    FieldType type;
    FieldValue value;
    bool integer(const char* name, std::uint32_t input) noexcept; // 4339C769
    bool floating(const char* name, float input) noexcept; // 433B21D7
    bool string(const char* name, const char* input) noexcept; // 4341848A
    bool boolean(const char*, bool) noexcept; // 43337B89
    bool wide_integer(const char*, std::uint64_t) const noexcept {
        return true;
    } // 433ADABB
};

// Engine event vtable +0x4C. True means the requested field could not be read;
// the original listeners return immediately on that result.
struct GameEventReader {
    virtual ~GameEventReader() = default;
    virtual bool read(std::uint32_t name_hash, FieldType type, FieldValue& value) = 0;
};
struct MutableGameEvent : GameEventReader {
    virtual void set_string(const char* key, const char* value) = 0; // event slot16
};
struct EventPlayerIndices {
    virtual ~EventPlayerIndices() = default;
    virtual std::int32_t player_for_user_id(std::uint32_t user_id) = 0;
    virtual std::int32_t local_player_index() = 0;
};

enum class ShotEventKind : std::uint32_t {
    fire,
    impact,
    hurt,
    death
};
// Native queue elements occupy 0x20 bytes. The listeners write payload_words()
// payload values; the remaining native append bytes are stack residue.
struct ShotEvent {
    ShotEventKind kind{};
    std::array<std::uint32_t, 7> payload{};
    std::uint32_t payload_words() const noexcept;
};
static_assert(sizeof(ShotEvent) == 0x20);

struct GameEventState {
    bool enabled = false; // native 0x4346754C
    bool has_pending_shots = false; // owner +0x4C != owner +0x50
    std::vector<ShotEvent> feedback; // owner +0xE0
};

// Complete native listener owners: 4338A387, 43400F91, 433B6725, 4341ADCB.
void weapon_fire(GameEventState&, GameEventReader&, EventPlayerIndices&);
void bullet_impact(GameEventState&, GameEventReader&, EventPlayerIndices&);
void player_hurt(GameEventState&, GameEventReader&, EventPlayerIndices&);
void player_death(GameEventState&, GameEventReader&, EventPlayerIndices&);

} // namespace skeet::events
