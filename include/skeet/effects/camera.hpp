#pragma once
#include "skeet/config.hpp"
#include <array>
#include <cstddef>

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::effects {

struct CameraNetvars {
    std::uint32_t life_state;
    std::uint32_t observer_mode;
    std::uint32_t observer_target;
    std::uint32_t fov;
    // The second cleared dword has no independently recovered field name.
    // Native resolver: *0x43468D10 ^ *0x43468DCC ^ 0x3F51AA0A.
    std::uint32_t reset_field_3f51aa0a;
};

struct SavedCameraState {
    std::int32_t fov = 0; // native player record +0xFC
    std::int32_t observer_mode = 0; // native player record +0x164
};
struct SavedCameraStateView {
    std::int32_t& fov;
    std::int32_t& observer_mode;
    SavedCameraStateView& operator=(SavedCameraState value) noexcept {
        fov = value.fov;
        observer_mode = value.observer_mode;
        return *this;
    }
};
// The original record lookup masks the requested index with 63 and initializes
// an absent record to zero. The owning player-state capture supplies these two
// fields before observer overrides; this camera code does not invent a capture.
struct CameraHistory {
private:
    client::PlayerStateCache* source_ = nullptr;

public:
    CameraHistory() = default;
    explicit CameraHistory(client::PlayerStateCache& source) noexcept : source_(&source) {}
    std::array<SavedCameraState, 64> players{};
    SavedCameraStateView at(std::uint32_t index);
};

// Native observer-property proxy argument layout. At 0x43394077..0x433940A2
// and 0x43394228..0x43394269 the integer is at +8 and tag 1 is at +0x14.
// The two unused dwords at +0xC/+0x10 are initialized here; native leaves them
// unspecified. The remaining eight dwords reproduce its explicit writes.
struct ObserverProxyData {
    std::array<std::uint32_t, 10> words{};
};
using ObserverProxy = void (*)(const ObserverProxyData*, void* entity, void* destination);

struct CameraInterfaces {
private:
    void* standalone_engine_ = nullptr;
    void* standalone_entities_ = nullptr;
    void* standalone_input_ = nullptr;
    void* standalone_local_ = nullptr;
    ObserverProxy standalone_proxy_ = nullptr;

public:
    CameraInterfaces() noexcept
        : engine(standalone_engine_), entity_list(standalone_entities_), input(standalone_input_),
          local_player(standalone_local_), observer_proxy(standalone_proxy_) {}
    explicit CameraInterfaces(void* engine_object) noexcept : CameraInterfaces() {
        engine = engine_object;
    }
    CameraInterfaces(void*& engine_cell, void*& entities_cell, void*& input_cell, void*& local_cell,
                     ObserverProxy& proxy_cell) noexcept
        : engine(engine_cell), entity_list(entities_cell), input(input_cell),
          local_player(local_cell), observer_proxy(proxy_cell) {}
    CameraInterfaces(const CameraInterfaces&) = delete;
    CameraInterfaces& operator=(const CameraInterfaces&) = delete;
    CameraInterfaces(CameraInterfaces&&) = delete;
    CameraInterfaces& operator=(CameraInterfaces&&) = delete;
    void*& engine; // native global 0x4346A514
    void*& entity_list; // 0x4346A890
    void*& input; // 0x4346A894
    void*& local_player; // 0x43475808
    ObserverProxy& observer_proxy; // 0x4347BDE4, optional
};

// Uses actual engine vtable entries, not caller-supplied substitute actions:
// engine[26], entity_list[4], player[158]/[312], networkable(+8)[10], and
// input[32]/[36]. Their byte offsets are the recovered x86 offsets / 4.
class CameraRuntime {
    bool standalone_forced_observer_ = false;

public:
    CameraRuntime(CameraInterfaces& interfaces, CameraNetvars offsets, CameraHistory& history,
                  const std::uint8_t& force_dead, const RegisteredHotkey& alive_hotkey)
        : interfaces(interfaces), offsets(offsets), history(history), force_dead(force_dead),
          alive_hotkey(alive_hotkey), forced_observer(standalone_forced_observer_) {}
    CameraRuntime(CameraInterfaces& interfaces, CameraNetvars offsets, CameraHistory& history,
                  const std::uint8_t& force_dead, const RegisteredHotkey& alive_hotkey,
                  bool& forced_observer)
        : interfaces(interfaces), offsets(offsets), history(history), force_dead(force_dead),
          alive_hotkey(alive_hotkey), forced_observer(forced_observer) {}
    CameraRuntime(const CameraRuntime&) = delete;
    CameraRuntime& operator=(const CameraRuntime&) = delete;
    CameraInterfaces& interfaces;
    CameraNetvars offsets;
    CameraHistory& history;
    const std::uint8_t& force_dead;
    const RegisteredHotkey& alive_hotkey;
    InputBitmap keys{};
    bool& forced_observer; // native global 0x4347CA59

    void alive_changed(bool enabled); // complete 0x43322683
    void dead_changed(); // complete 0x433662E6
    void update_observer(void* player); // complete 0x43393F51, valid entity-index contract
private:
    bool alive_or_death_camera(void* player) const; // 0x43386EB3
    void apply_observer_mode(void* player, std::int32_t mode);
    void reset_camera(void* player);
};

} // namespace skeet::effects
