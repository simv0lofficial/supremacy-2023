#include "skeet/client/source_dll.hpp"
#include "skeet/client/source_application.hpp"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <exception>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace {
using skeet::client::SourceApplication;
using skeet::client::SourceDllStatus;
using skeet::client::SourceDllShutdownReadiness;

std::atomic<SourceDllStatus> status{SourceDllStatus::initializing};
std::atomic<SourceApplication*> application{nullptr};
char startup_error[4096]{};

SourceDllShutdownReadiness shutdown_readiness(void*) noexcept {
    switch (status.load(std::memory_order_acquire)) {
    case SourceDllStatus::initializing:
        return SourceDllShutdownReadiness::waiting;
    case SourceDllStatus::ready:
        return SourceDllShutdownReadiness::ready;
    default:
        return SourceDllShutdownReadiness::failed;
    }
}
void finish_application(void*) noexcept {
    status.store(SourceDllStatus::stopping, std::memory_order_release);
    delete application.exchange(nullptr, std::memory_order_acq_rel);
    status.store(SourceDllStatus::stopped, std::memory_order_release);
}
void publish_failure(const char* reason) noexcept {
    std::snprintf(startup_error, sizeof(startup_error), "%s", reason);
    // Publish failure before cancelling a pending UI unload request. The
    // request gate must never admit new teardown during startup rollback.
    status.store(SourceDllStatus::initialization_failed, std::memory_order_release);
    OutputDebugStringA("skeet_client: initialization failed: ");
    OutputDebugStringA(startup_error);
    OutputDebugStringA("\n");
    auto* failed = application.load(std::memory_order_acquire);
    if (failed) {
        if (failed->cancel_initialization()) {
            delete application.exchange(nullptr, std::memory_order_acq_rel);
        } else {
            status.store(SourceDllStatus::cleanup_failed, std::memory_order_release);
            OutputDebugStringA(
                "skeet_client: partial state retained because startup cleanup could not complete\n");
        }
    }
    MessageBoxA(nullptr, startup_error, "skeet_client - initialization failed",
                MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
}
DWORD WINAPI initialize_source_application(void* loaded_module) noexcept {
    HMODULE worker_reference = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                            reinterpret_cast<LPCWSTR>(&initialize_source_application),
                            &worker_reference)) {
        char message[128];
        std::snprintf(message, sizeof(message),
                      "Cannot retain the source initialization thread module (Win32 %lu)",
                      static_cast<unsigned long>(GetLastError()));
        publish_failure(message);
        return 1;
    }
    DWORD result = 1;
    try {
        auto* created =
            new SourceApplication(loaded_module, {nullptr, finish_application, shutdown_readiness});
        application.store(created, std::memory_order_release);
        created->initialize(reinterpret_cast<std::uintptr_t>(loaded_module));
        // This is the last application access on the worker. Publishing ready
        // admits deferred menu unload; its completion may now delete created.
        status.store(SourceDllStatus::ready, std::memory_order_release);
        result = 0;
    } catch (const std::exception& error) {
        publish_failure(error.what());
    } catch (...) {
        publish_failure("Source initialization raised an unknown C++ exception");
    }
    // The worker owns a separate loader reference. Its final release and exit
    // are atomic with respect to code unmapping after a concurrent menu unload.
    FreeLibraryAndExitThread(worker_reference, result);
}
}

extern "C" std::uint32_t __cdecl SkeetGetStatus() noexcept {
    return static_cast<std::uint32_t>(status.load(std::memory_order_acquire));
}
extern "C" std::uint32_t __cdecl SkeetCopyStartupError(char* buffer,
                                                       std::uint32_t capacity) noexcept {
    const auto current = status.load(std::memory_order_acquire);
    const char* text = (current == SourceDllStatus::initialization_failed ||
                        current == SourceDllStatus::cleanup_failed)
                           ? startup_error
                           : "";
    const auto length = static_cast<std::uint32_t>(std::strlen(text));
    if (buffer && capacity) {
        const auto copied = length < capacity ? length : capacity - 1;
        std::memcpy(buffer, text, copied);
        buffer[copied] = '\0';
    }
    return length + 1;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, void*) {
    if (reason != DLL_PROCESS_ATTACH)
        return TRUE;
    // Only create the worker under the loader lock. Engine discovery, CRT
    // callbacks, owner creation and hook installation run after DllMain exits.
    const auto thread = CreateThread(nullptr, 0, initialize_source_application, module, 0, nullptr);
    if (!thread)
        return FALSE;
    CloseHandle(thread);
    return TRUE;
}
