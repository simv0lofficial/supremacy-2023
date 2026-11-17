#pragma once
#include "source_dll_shutdown.hpp"
#include <cstdint>
#include <memory>

namespace skeet::client {
class ClientRuntimeStorage;
// Complete process lifetime for the source DLL. Construction prepares the
// early owners; initialize() executes their CRT schedule and the actual final
// engine/menu/hook operation. The caller retains this object until the source
// shutdown completion callback, after engine hooks and callbacks are removed.
class SourceApplication final {
public:
    SourceApplication(void* source_module, SourceDllShutdownCompletion);
    ~SourceApplication();
    SourceApplication(const SourceApplication&) = delete;
    SourceApplication& operator=(const SourceApplication&) = delete;
    SourceApplication(SourceApplication&&) = delete;
    SourceApplication& operator=(SourceApplication&&) = delete;
    void initialize(std::uintptr_t initialization_argument);
    bool initialized() const noexcept;
    // Called only by the initialization thread after publishing a failed
    // startup state. Failure retains this application and its DLL: cleanup
    // must never dispatch an incomplete owner or repeat partial destruction.
    bool cancel_initialization() noexcept;
    ClientRuntimeStorage& storage() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
