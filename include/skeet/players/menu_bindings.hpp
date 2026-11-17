#pragma once
#include "state.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
}
namespace skeet::players {
class MenuBindings final {
public:
    MenuBindings(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&);
    MenuBindings(const MenuBindings&) = delete;
    MenuBindings& operator=(const MenuBindings&) = delete;
    Bindings& bindings() noexcept {
        return bindings_;
    }

private:
    LiveBindings live_;
    Bindings bindings_;
};
} //namespace skeet::players
