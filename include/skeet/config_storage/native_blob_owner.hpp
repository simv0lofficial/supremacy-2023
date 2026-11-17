#pragma once
#include "skeet/config.hpp"
#include "skeet/config_storage/native_blob.hpp"

namespace skeet {
// The32-byte dynamic string allocation from433CBCD0: a physical native
// header followed immediately by its three32-bit buffer pointers. The source
// ConfigBlob is only its typed view and is never inserted in the native ring.
// Fields occupy28 bytes; the native allocator request is32, including its
// untouched trailing dword. Caller433ECDEE unlinks the header before deletion.
class NativeConfigBlobOwner {
public:
    static std::unique_ptr<NativeConfigBlobOwner> create(ConfigRegistries&, engine::JobMemory,
                                                         std::byte& actual_shared_empty,
                                                         std::uint32_t hash);
    ~NativeConfigBlobOwner();
    NativeConfigBlobOwner(const NativeConfigBlobOwner&) = delete;
    NativeConfigBlobOwner& operator=(const NativeConfigBlobOwner&) = delete;
    ConfigBlob& configuration() noexcept {
        return configuration_;
    }
    ConfigDescriptor& native_header() noexcept {
        return header_;
    }

private:
    NativeConfigBlobOwner(ConfigRegistries&, engine::JobMemory, ConfigDescriptor&, NativeBlobCells&,
                          std::byte&);
    ConfigRegistries& registry_;
    engine::JobMemory memory_;
    ConfigDescriptor& header_;
    NativeBlobBytes bytes_;
    ConfigBlob configuration_;
};
} // namespace skeet
