#include "panorama_detail.hpp"

namespace skeet::script {
using namespace panorama_detail;

bool PanoramaModule::unsafe_enabled() const { // 0x433466CC
    if (!services_.unsafe_43475798 || !services_.unsafe_43475a5c || !services_.unsafe_4347582c)
        throw std::logic_error("Panorama requires all three native unsafe flags");
    return *services_.unsafe_43475798 || *services_.unsafe_43475a5c || *services_.unsafe_4347582c;
}

void* PanoramaModule::ui_engine() const {
    if (!services_.engine_service)
        throw std::logic_error("Panorama requires engine service 43468E98");
    return engine_call<void*>(services_.engine_service, 11);
}

void* PanoramaModule::find_panel(const char* name) const {
    auto* engine = ui_engine();
    if (!engine)
        return nullptr;
    const auto panels = panel_tree(engine);
    for (std::int32_t index = 0; index < panels.allocated; ++index) {
        if (!panels.valid(static_cast<std::uint32_t>(index)))
            continue;
        auto* panel = panels.records[index].panel;
        if (!panel)
            continue;
        const auto* id = engine_call<const char*>(panel, 9);
        if (id && std::strcmp(id, name) == 0)
            return panel;
    }
    return nullptr;
}

void* PanoramaModule::default_panel() const {
    auto* engine = ui_engine();
    if (!engine)
        return nullptr;
    const auto panels = panel_tree(engine);
    return panels.valid(0) ? panels.records[0].panel : nullptr;
}

void*** PanoramaModule::panel_context(void* engine, void* panel) const {
    if (!panel || !engine_call<bool>(engine, 36, panel))
        return nullptr;
    auto* context = engine_call<void***>(engine, 120, panel);
    return context && *context ? context : nullptr;
}

int PanoramaModule::take_handle(void* isolate, V8Local value, bool script) {
    auto& handles = script ? browser_.script_handles : browser_.object_handles;
    auto& free = script ? browser_.free_script_indices : browser_.free_object_indices;
    std::uint32_t index;
    if (free.empty()) {
        index = static_cast<std::uint32_t>(handles.size());
        handles.push_back(nullptr);
    } else {
        index = free.back();
        free.pop_back();
        if (handles[index]) {
            services_.v8.dispose_global(handles[index]);
            handles[index] = nullptr;
        }
    }
    if (value.handle)
        handles[index] = services_.v8.globalize_reference(isolate, value.handle);
    return static_cast<int>(index);
}

} // namespace skeet::script
