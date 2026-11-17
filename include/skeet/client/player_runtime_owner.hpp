#pragma once
#include "player_state.hpp"
#include "skeet/players/state.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
// One cache, record pool and player-adjustment view shared by menu, RAGE,
// animation, Lua and history consumers. Source terminal cleanup receives the
// same cache/pool; neither construction nor destruction runs CRT callbacks.
class PlayerRuntimeOwner {
public:
    explicit PlayerRuntimeOwner(ClientRuntimeStorage&);
    ~PlayerRuntimeOwner();
    PlayerRuntimeOwner(const PlayerRuntimeOwner&) = delete;
    PlayerRuntimeOwner& operator=(const PlayerRuntimeOwner&) = delete;
    PlayerRuntimeOwner(PlayerRuntimeOwner&&) = delete;
    PlayerRuntimeOwner& operator=(PlayerRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    PlayerStateCache& cache() noexcept {
        return cache_;
    }
    players::State& adjustments() const noexcept {
        return adjustments_;
    }
    lagcomp::RecordPool& records() noexcept {
        return records_;
    }

private:
    static ClientRuntimeStorage& require_available(ClientRuntimeStorage&);
    ClientRuntimeStorage& storage_;
    lagcomp::RecordPool records_;
    PlayerStateCache cache_;
    players::State& adjustments_;
};
} //namespace skeet::client
