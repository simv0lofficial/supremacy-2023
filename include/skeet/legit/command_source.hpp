#pragma once
#include "command.hpp"

namespace skeet::legit {
// Production owns the process dependencies and forms the physical binding
// graph from the current phase's player/frame. No initial fake player or
// retained Command is needed to construct/register the lifecycle owner.
class CommandSource {
public:
    virtual ~CommandSource() = default;
    virtual void clear_allocated_storage() = 0;
    virtual void capture_view(RuntimeState&) = 0;
    virtual void process(RuntimeState&, const RuntimeSettings&, client::CommandFrame&, void*,
                         client::Command&) = 0;
    virtual void reset(RuntimeState&) = 0;
    virtual void pending(RuntimeState&, client::Command&, float) = 0;
};
} //namespace skeet::legit
