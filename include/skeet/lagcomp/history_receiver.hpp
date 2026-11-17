#pragma once
#include "history_runtime.hpp"
#include "skeet/client/entity_observer_receiver.hpp"

namespace skeet::lagcomp {
// The four-entry interface registered by4333BF32, whose original table is
//43471D6C. The receiver lives inside its complete HistoryOwnerRuntime and is
//destroyed with that owner. Production recording owns automatic scratch;
//the second constructor is an explicit differential-fixture input seam.
struct HistoryRecordingReceiver : client::NativeEntityObserver {
    HistoryRecordingRuntime* runtime;
    HistoryRecordingScratch* scratch;
    // The enclosing owner performs registration and later table publication
    // at their distinct native constructor cuts, after binding these views.
    explicit HistoryRecordingReceiver(HistoryRecordingRuntime&) noexcept;
    HistoryRecordingReceiver(HistoryRecordingRuntime&, HistoryRecordingScratch&) noexcept;
    void record_players(void* local_player) const;
};
} //namespace skeet::lagcomp
