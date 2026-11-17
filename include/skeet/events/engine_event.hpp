#pragma once
#include "game_events.hpp"
#include "hit_log.hpp"

namespace skeet::events {
// GameEvent slot19 consumes this seven-slot interface, without a destructor
// slot. Native typed visitor4346F114 and Lua visitor43470434 agree on argument
// sizes: slot4 pops12 bytes; all other slots pop8 bytes on x86.
struct EngineEventVisitor {
    virtual bool pointer(const char* key, void* value) = 0;
    virtual bool string(const char* key, const char* value) = 0;
    virtual bool floating(const char* key, float value) = 0;
    virtual bool integer(const char* key, std::int32_t value) = 0;
    virtual bool wide_integer(const char* key, std::uint64_t value) = 0;
    virtual bool opaque_word(const char* key, std::uint32_t value) = 0;
    virtual bool boolean(const char* key, bool value) = 0;
};

// The actual engine event boundary; typed listeners and Lua dispatch consume
// this same event object. The engine owns its lifetime.
class EngineGameEventReader final : public MutableGameEvent {
public:
    explicit EngineGameEventReader(void* event) noexcept : event_(event) {}
    void* raw_event() const noexcept {
        return event_;
    }
    const char* name() const; // event slot1
    bool read(std::uint32_t name_hash, FieldType type, FieldValue& value) override;
    void set_string(const char* key, const char* value) override;
    void set_boolean(const char* key, bool value); // event slot13
private:
    void* event_;
};

// Hashed callbacks receive the local entity captured after the pre-event pass.
// Native comparisons use IClientNetworkable::EntIndex at local+8, slot10.
class EngineEventPlayers final : public HitLogPlayerInfo {
public:
    EngineEventPlayers(void* engine, void* local_player) noexcept
        : engine_(engine), local_(local_player) {}
    void* local_entity() const noexcept {
        return local_;
    }
    std::int32_t player_for_user_id(std::uint32_t user_id) override;
    std::int32_t local_player_index() override;
    bool player_info(std::int32_t index, std::array<std::byte, 0x158>& output) override;

private:
    void* engine_;
    void* local_;
};
} // namespace skeet::events
