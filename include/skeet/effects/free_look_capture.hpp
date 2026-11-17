#pragma once
#include "free_look_view.hpp"

namespace skeet::effects {
class FreeLookOwner;

struct FreeLookCaptureBindings {
    void* const& input; //4346A894; actual slots32/34, camera distance+B4
    void* const& engine_trace; //43468304; actual TraceRay slot5
    void* const& engine_info; //43467014; actual slot3 trace-entity predicate
    void* const& entity_list; //4346A890; actual handle lookup slot4
    void* const* const& trace_filter_vtable; //genuine external table434758B4
    std::uint32_t owner_handle; //decoded4E36DFD7
    FreeLookOwner* const& free_look; //actual43475A64 owner, required at capture time
    std::int32_t* const& frame; //434678F8, pointed engine frame
    RageTrigonometry trigonometry; //actual imported SSE sine/cosine
};

// Complete4337EE59, including4333D32B. receiver is the native-compatible
// filter prefix passed to the genuine engine filter's first virtual method.
// Resolved entities and every owner are rejected by actual slots162/158.
bool free_look_capture_accepts(void* receiver, void* candidate, std::uint32_t mask,
                               const FreeLookCaptureBindings&);

// Complete433CCEA1..433CD183. The saved original player vtable slot337
// (byte offset0x544) runs first. The input camera, the special owner-chain trace
// filter, distance adjustment and unconditional origin/frame capture stay here.
// setup is the engine view setup with origin at+B8 and height component+C0.
void capture_free_look_view_hook(void* player, void** original_table, void* setup,
                                 const FreeLookCaptureBindings&);

} //namespace skeet::effects
