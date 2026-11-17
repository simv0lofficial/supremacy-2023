#pragma once
#include "target_mask_lifecycle.hpp"

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
struct EngineRuntimeBindings;
class TargetMaskRuntimeOwner final {
public:
    TargetMaskRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                           ConfigurationRuntimeOwner&);
    TargetMaskRuntimeOwner(const TargetMaskRuntimeOwner&) = delete;
    TargetMaskRuntimeOwner& operator=(const TargetMaskRuntimeOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    TargetMaskOwner& command() noexcept {
        return command_;
    }
    TargetMaskFactoryBindings& factory_bindings() noexcept {
        return factories_;
    }
    void bind_initializers(NativeStartupSources&);

private:
    ClientRuntimeStorage& storage_;
    TargetMaskOwner command_;
    TargetMaskFactoryBindings factories_;
};
}
