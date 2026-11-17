#pragma once
#include "vector.hpp"
#include <string_view>
namespace skeet::script {
class CompatModule {
public:
    CompatModule(Bridge& bridge, VectorModule::LoadBuffer loadbuffer,
                 bool& loading_builtin) noexcept
        : bridge_(bridge), loadbuffer_(loadbuffer), loading_builtin_(loading_builtin) {}
    // Native 433E8EE0/4333FC10: leaves one runtime table on success.
    int open();
    static std::string_view source() noexcept;

private:
    Bridge& bridge_;
    VectorModule::LoadBuffer loadbuffer_;
    bool& loading_builtin_;
};
}
