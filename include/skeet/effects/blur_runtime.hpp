#pragma once
#include "skeet/effects/blur.hpp"
#include "skeet/engine/owner_registry.hpp"

namespace skeet::effects {
#if defined(_M_IX86) || defined(__i386__)
using EffectFrameEntry = void(__fastcall*)(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t,
                                           std::uintptr_t);
#else
using EffectFrameEntry = void (*)(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t,
                                  std::uintptr_t);
#endif
// Native registered callback objects have one frame entry. ECX receives the
// object, EDX receives the first forwarded word, and three words stay on stack.
struct EffectFrameCallback {
    const EffectFrameEntry* vtable;
};

// Complete433B8C46. The registry is43475A84/88/8C, using the same pointer-vector
// implementation as the canonical owner registry. Its begin/end are captured
// once; callback registration during dispatch does not extend that pass.
void dispatch_effect_frame(const engine::OwnerRegistry& callbacks, std::uintptr_t first,
                           std::uintptr_t second, std::uintptr_t third, std::uintptr_t fourth);

// Complete4347F6DB/4347E37A composition. active is the canonical43475A20 slot.
// Registration runs in native constructor order: lifecycle owner first, frame
// endpoint second, then vtables and effect state. The owner registry's slot0
// destroys the instance and clears active; slot4 releases render resources.
// Other lifecycle entries are the actual native433CCE8E return-only body.
void initialize_blur(engine::OwnerRegistryBinding owners, engine::OwnerRegistryBinding callbacks,
                     BlurEffect*& active, BlurBindings bindings);
} // namespace skeet::effects
