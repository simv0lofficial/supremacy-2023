#pragma once
#include "skeet/client/entity_command_owner.hpp"
#include "skeet/engine/native_data_image.hpp"

namespace skeet::client::entity_command_detail {
// One concrete uint32->uint32 native map at full owner+74. The view is the
// exact table borrowed by EntityCommandState::model_categories.
class ModelCategories {
public:
    ModelCategories(engine::JobMemory memory, const engine::NativeDataImage& image,
                    double (*ceiling)(double))
        : memory_(memory), image_(image), ceiling_(ceiling) {}
    void initialize() noexcept;
    void parse(const char* entity_text); //4339D139
    void release(); //433848F0
    const EntityModelCategories& view() const noexcept {
        return view_;
    }

private:
    engine::JobMemory memory_;
    const engine::NativeDataImage& image_;
    double (*ceiling_)(double);
    EntityModelCategories view_;
    std::uint32_t count_ = 0, next_resize_ = 0;
    float maximum_load_ = 1.0f, multiplier_ = 2.0f;
    void clear(); //433CF5BD
    EntityModelCategoryNode& find_or_insert(const std::uint32_t*); //43401105
    void rehash(std::uint32_t); //4334C59A
};
} // namespace skeet::client::entity_command_detail
