#pragma once

namespace skeet::client {
class ClientRuntimeStorage;
class WindowRuntimeOwner;
// Read-only preflight for cancellation of source initialization. Failure
// requires retaining the application; these checks perform no teardown.
bool source_exit_callbacks_ready(const ClientRuntimeStorage&) noexcept;
bool source_startup_detach_ready(ClientRuntimeStorage&, WindowRuntimeOwner&) noexcept;
} //namespace skeet::client
