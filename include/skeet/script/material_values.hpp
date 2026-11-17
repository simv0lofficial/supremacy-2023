#pragma once
#include <cstdint>

namespace skeet::script {
class MaterialModule;
class MaterialValues {
public:
    static void set(MaterialModule&, void* state, void* variable, int value_index); // 43400987
    static int get(MaterialModule&, void* state, void* variable); // 433D66C3
    static void* create(MaterialModule&, void* material, const char* name); // engine43467A68
private:
    static std::uint32_t tag(MaterialModule&, void* state, int index); // 433203AB
};
} // namespace skeet::script
