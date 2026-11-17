#pragma once
#include "entity_overlay.hpp"
#include "skeet/engine/jobs.hpp"
#include <array>
#include <iterator>

namespace skeet::effects {
class GlowRuntime;
}
namespace skeet::esp {
struct EspFrameOwner;
// Specialized native4340B322 map: uint16 model index -> shared constructor
// descriptor. Policy is the actual owner defaults, load1 and growth2.
class DangerZoneModels {
public:
    struct Entry {
        std::uint16_t model_index;
        DangerZoneItem* item;
    };

private:
    struct Node {
        Entry entry;
        Node* next;
    };

public:
    class Iterator {
    public:
        using value_type = Entry;
        using pointer = const Entry*;
        using reference = const Entry&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        Iterator() = default;
        const Entry& operator*() const noexcept {
            return node_->entry;
        }
        const Entry* operator->() const noexcept {
            return &node_->entry;
        }
        Iterator& operator++() noexcept;
        Iterator operator++(int) noexcept {
            auto previous = *this;
            ++*this;
            return previous;
        }
        friend bool operator==(const Iterator&, const Iterator&) = default;

    private:
        friend class DangerZoneModels;
        Iterator(const DangerZoneModels* map, std::uint32_t bucket, Node* node)
            : map_(map), bucket_(bucket), node_(node) {}
        const DangerZoneModels* map_ = nullptr;
        std::uint32_t bucket_ = 0;
        Node* node_ = nullptr;
    };
    explicit DangerZoneModels(engine::JobMemory memory = engine::native_job_memory());
    ~DangerZoneModels();
    DangerZoneModels(const DangerZoneModels&) = delete;
    DangerZoneModels& operator=(const DangerZoneModels&) = delete;
    bool insert(std::uint16_t, DangerZoneItem&); //4340B322, duplicates retain first
    DangerZoneItem* find(std::uint16_t) const noexcept;
    void clear(); //433CF5BD: retain buckets and next-resize policy
    void release_storage(); //433848F0, at the native owner destruction point
    Iterator begin() const noexcept; //4340C443
    Iterator end() const noexcept {
        return {this, bucket_count_, nullptr};
    }
    std::uint32_t size() const noexcept {
        return size_;
    }
    std::uint32_t bucket_count() const noexcept {
        return bucket_count_;
    }

private:
    void* allocate(std::size_t);
    void free(void*);
    void rehash(std::uint32_t); //43384D3A
    engine::JobMemory memory_;
    Node* initial_ = nullptr;
    Node** buckets_ = &initial_;
    std::uint32_t bucket_count_ = 1, size_ = 0, next_resize_ = 0;
};
struct EntityModelState {
    explicit EntityModelState(engine::JobMemory memory = engine::native_job_memory());
    std::array<DangerZoneItem, 20> descriptors;
    DangerZoneModels models;
    void bind_config(const WorldEspConfig&) noexcept;
};
void reset_danger_zone_models(EntityModelState&) noexcept;
void collect_danger_zone_model(EntityModelState&, const WorldEntity&,
                               Vector3 local_origin) noexcept;
void draw_danger_zone_models(ui::RenderQueue&, const Projection&, std::int32_t game_mode,
                             bool enabled, const EntityModelState&);
// Complete43343465 and433E2C67. Glow fields and sound model belong to this
// same native owner; its kit-model index survives reset until a new match.
void reset_esp_frame_owner(EspFrameOwner&, effects::GlowRuntime&);
void initialize_esp_entity_models(EspFrameOwner&, effects::GlowRuntime&, const WorldEspConfig&,
                                  void* const& network_string_tables);
} // namespace skeet::esp
