#pragma once
#include "history_runtime_owner.hpp"
#include "misc_runtime_owner.hpp"
#include "packet_runtime_owner.hpp"
#include "skeet/anti_aim/runtime_owner.hpp"
#include "skeet/script/api_runtime_owner.hpp"

namespace skeet::client {
// Explicit external services for an authored integrated boundary fixture.
// Every recovered domain owner remains the production implementation. The
// history and RAGE dispatchers receive this same actual external thread pool.
struct SourceFeatureRuntimeImports {
    HistoryRuntimeImports history;
    MiscRuntimeImports miscellaneous;
    PacketRuntimeImports packet;
    anti_aim::RuntimeImports anti_aim;
    script::ScriptApiRuntimeImports script_api;
};
} //namespace skeet::client
