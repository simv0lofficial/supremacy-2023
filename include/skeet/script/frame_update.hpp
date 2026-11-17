#pragma once
#include "skeet/engine/frame_update.hpp"

namespace skeet::script {
class ScriptManager;

// Native owner+30, vtable43472490. The forwarded DL byte is unused by
// 4338174B; the manager checks the Lua state before consuming4347CA28.
class ScriptFrameUpdate final : public engine::FrameUpdateEndpoint {
public:
    ScriptFrameUpdate(ScriptManager&, std::uint8_t& reload_requested,
                      engine::OwnerRegistryBinding frames);

private:
    ScriptManager& manager_;
    std::uint8_t& reload_requested_;
    static const engine::FrameUpdateEntry entries_[1];
#if defined(_M_IX86) || defined(__i386__)
    static void __fastcall update_entry(void*, std::uint8_t);
#else
    static void update_entry(void*, std::uint8_t);
#endif
};
} // namespace skeet::script
