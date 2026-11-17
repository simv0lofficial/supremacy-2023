#pragma once
#include "skeet/script/initialize.hpp"

namespace skeet::script {

// Owns the45 LuaJIT bytecode modules recovered from434792D0. The captured
//434792E0 builtin list was empty. Each process encrypts its own mutable copies
// with its actual process cipher before the ScriptOwner factory can use them.
class RequiredModuleStorage {
public:
    static constexpr std::size_t module_count = 45;
    static constexpr std::size_t source_size = 449723;
    RequiredModuleStorage();
    RequiredModuleStorage(const RequiredModuleStorage&) = delete;
    RequiredModuleStorage& operator=(const RequiredModuleStorage&) = delete;
    RequiredModuleStorage(RequiredModuleStorage&&) = delete;
    RequiredModuleStorage& operator=(RequiredModuleStorage&&) = delete;

    std::span<RequiredModule> modules() noexcept {
        return modules_;
    }
    std::span<const RequiredModule> modules() const noexcept {
        return modules_;
    }
    std::span<BuiltinModule> builtins() const noexcept {
        return {};
    }
    void encrypt_for(ScriptRuntime&);

private:
    std::vector<RequiredModule> modules_;
    bool encrypted_ = false;
};
} //namespace skeet::script
