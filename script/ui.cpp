#include "skeet/script/ui.hpp"
#include "detail.hpp"
#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <type_traits>

namespace skeet::script {
namespace {
constexpr int registry = -10000;
using namespace detail;
void require_state(const Bridge& bridge) {
    if (!bridge.api || !bridge.state)
        throw std::logic_error("Lua UI requires an initialized state");
}
ConfigDescriptor* descriptor(UiConfiguration& configuration) {
    return std::visit(
        [](auto& value) -> ConfigDescriptor* {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::monostate>)
                return nullptr;
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>,
                                              std::unique_ptr<NativeConfigBlobOwner>>)
                return value ? &value->configuration() : nullptr;
            else
                return value.get();
        },
        configuration);
}
void unlink_config(ConfigDescriptor& descriptor) {
    descriptor.previous->next = descriptor.next;
    descriptor.next->previous = descriptor.previous;
}
}

std::uint32_t UiBindings::reference(ui::Widget& widget) { // 0x43419FFC
    for (std::uint32_t index = 0; index < records.size(); ++index)
        if (records[index].borrowed && records[index].widget == &widget)
            return index;
    auto& binding = records.emplace_back();
    binding.widget = &widget;
    binding.borrowed = true;
    return static_cast<std::uint32_t>(records.size() - 1);
}

std::uint32_t UiBindings::adopt(std::unique_ptr<ui::Widget> widget, UiConfiguration configuration) {
    auto& binding = records.emplace_back();
    binding.widget = widget.get();
    binding.configuration = descriptor(configuration);
    binding.owned_widget = std::move(widget);
    binding.owned_configuration = std::move(configuration);
    return static_cast<std::uint32_t>(records.size() - 1);
}

void UiBindings::initialize(Bridge& bridge) {
    require_state(bridge);
    records.reserve(std::max<std::size_t>(records.capacity(), 128));
    callback_active_.clear();
    const auto& api = *bridge.api;
    api.createtable(bridge.state, 0, 0);
    callbacks_reference = api.ref(bridge.state, registry);
    api.createtable(bridge.state, 0, 0);
    values_reference = api.ref(bridge.state, registry);
}

void UiBindings::widget_changed(void* context, ui::Widget&) {
    auto& callback = *static_cast<CallbackContext*>(context);
    callback.bindings->dispatch_callback(*callback.bridge, callback.reference);
}

void UiBindings::remove_widget_callback(UiBinding& record, int reference) {
    if (!record.widget)
        return;
    auto& callbacks = record.widget->callbacks;
    const auto found =
        std::find_if(callbacks.begin(), callbacks.end(), [&](const ui::WidgetCallback& entry) {
            if (entry.context == reinterpret_cast<void*>(static_cast<std::intptr_t>(reference)))
                return true;
            return std::any_of(
                callback_contexts_.begin(), callback_contexts_.end(), [&](const auto& context) {
                    return entry.context == context.get() && context->reference == reference;
                });
        });
    if (found != callbacks.end())
        callbacks.erase(found);
}

bool UiBindings::set_callback(Bridge& bridge, std::uint32_t handle, int function_index,
                              void* call_state) { // 0x433F904C
    require_state(bridge);
    if (handle >= records.size() || !records[handle].widget)
        return false;
    const auto& api = *bridge.api;
    auto* state = call_state ? call_state : bridge.state;
    api.checktype(state, function_index, 6);
    const auto absolute_index = function_index < 0 && function_index > registry
                                    ? api.gettop(state) + function_index + 1
                                    : function_index;
    auto& record = records[handle];
    if (record.borrowed && record.widget->callbacks.size() > 1024)
        return true;
    if (!record.borrowed) {
        for (int& reference : record.callback_references) {
            if (reference == -1)
                continue;
            api.rawgeti(state, registry, values_reference);
            api.pushnil(state);
            api.rawseti(state, -2, reference);
            api.settop(state, -2);
            api.rawgeti(state, registry, callbacks_reference);
            api.unref(state, -1, reference);
            reference = -1;
            api.settop(state, -2);
        }
        record.callback_references.clear();
        record.widget->callbacks.clear();
    }
    const auto callback = create_callback(bridge, handle, absolute_index, state);
    record.callback_references.push_back(callback.reference);
    record.widget->callbacks.push_back(callback.callback);
    return true;
}

UiBindings::CallbackRegistration UiBindings::create_callback(Bridge& bridge, std::uint32_t handle,
                                                             int function_index, void* call_state) {
    require_state(bridge);
    const auto& api = *bridge.api;
    auto* state = call_state ? call_state : bridge.state;
    const auto absolute_index = function_index < 0 && function_index > registry
                                    ? api.gettop(state) + function_index + 1
                                    : function_index;
    api.rawgeti(state, registry, callbacks_reference);
    api.pushvalue(state, absolute_index);
    const auto reference = api.ref(state, -2);
    api.settop(state, -2);
    api.rawgeti(state, registry, values_reference);
    api.pushinteger(state, handle);
    api.rawseti(state, -2, reference);
    api.settop(state, -2);
    if (reference >= static_cast<int>(callback_active_.size()))
        callback_active_.resize(reference + 1);
    auto context = std::make_unique<CallbackContext>(CallbackContext{this, &bridge, reference});
    const ui::WidgetCallback callback{context.get(), widget_changed};
    callback_contexts_.push_back(std::move(context));
    return {reference, callback};
}

void UiBindings::dispatch_callback(Bridge& bridge, int reference) { // 0x4339CD77
    const std::lock_guard lock(bridge.vm_mutex);
    if (!bridge.state || callbacks_reference == -1 || values_reference == -1)
        return;
    require_state(bridge);
    if (reference < 0 || static_cast<std::size_t>(reference) >= callback_active_.size())
        throw std::out_of_range("Lua UI callback reference is not registered");
    if (callback_active_[reference])
        return;
    callback_active_[reference] = 1;
    const auto& api = *bridge.api;
    auto* state = bridge.state;
    CallbackFrame guard{};
    guard.active.flags = 0x67736c6a;
    guard.saved_top = api.gettop(state);
    RtlPushFrame(&guard.active);
    auto invoke = [&] {
        api.rawgeti(state, registry, callbacks_reference);
        api.rawgeti(state, -1, reference);
        api.remove(state, -2);
        api.rawgeti(state, registry, values_reference);
        api.rawgeti(state, -1, reference);
        api.remove(state, -2);
        if (callback_pcall(state, 1, 0) != 0)
            report_callback_error(bridge, state);
    };
    const bool dispatched = protected_region(invoke, guard);
    api.settop(state, guard.saved_top);
    RtlPopFrame(&guard.active);
    callback_active_[reference] = 0;
    if (!dispatched)
        report_native_exception(bridge, guard, "dispatching a Lua UI callback", true);
}

void UiBindings::clear_callbacks(Bridge& bridge) { // 0x433413B5
    require_state(bridge);
    const auto& api = *bridge.api;
    api.rawgeti(bridge.state, registry, callbacks_reference);
    for (auto& record : records) {
        for (int& reference : record.callback_references) {
            if (record.borrowed && record.widget)
                remove_widget_callback(record, reference);
            if (reference != -1) {
                api.unref(bridge.state, -1, reference);
                reference = -1;
            }
        }
        record.callback_references.clear();
    }
    api.settop(bridge.state, -2);
}

void UiBindings::release_callbacks(Bridge& bridge) { // 0x433B86C2
    clear_callbacks(bridge);
    release_reference(*bridge.api, bridge.state, values_reference);
    release_reference(*bridge.api, bridge.state, callbacks_reference);
}

void UiBindings::destroy_bindings() { // 0x433ECDEE, including 0x4338AB2D
    for (auto& record : records) {
        if (record.borrowed)
            continue;
        ui::Tab* tab = nullptr;
        if (record.widget) {
            tab = ui::owning_tab(record.widget);
            if (record.widget->kind == 9) {
                auto& hotkey = static_cast<HotkeyConfig*>(record.configuration)->value;
                hotkey.previous->next = hotkey.next;
                hotkey.next->previous = hotkey.previous;
            }
            if (window) {
                bool removed = false;
                for (auto* page : window->tabs) {
                    for (auto* category : page->categories) {
                        // Native search is backwards: repeated pointers remove
                        // the last occurrence, then stop searching other groups.
                        auto found = std::find(category->elements.rbegin(),
                                               category->elements.rend(), record.widget);
                        if (found == category->elements.rend())
                            continue;
                        {
                            std::unique_lock lock(category->elements_mutex);
                            category->elements.erase(std::next(found).base());
                            record.owned_widget.reset();
                            page->reflow_categories();
                        }
                        removed = true;
                        break;
                    }
                    if (removed)
                        break;
                }
            }
        }
        if (record.configuration) {
            unlink_config(config_native_header(*record.configuration));
            record.owned_configuration.emplace<std::monostate>();
        }
        if (tab)
            tab->focus_widget(nullptr);
    }
    for (auto& record : records) {
        // Native frees callback vector storage in a second pass, including
        // borrowed records, after all widget/configuration destruction.
        std::vector<int>().swap(record.callback_references);
    }
    records.clear();
    callback_contexts_.clear();
}
void UiBindings::finalize_record_storage() {
    // Native4337EA0A only destroys callback-reference vectors and their outer
    //record allocation. Runtime close must already have detached callbacks
    //and destroyed the widgets/configurations owned by these source records.
    if (!callback_contexts_.empty())
        throw std::logic_error("UI terminal storage cleanup requires destroyed callback bindings");
    for (const auto& record : records)
        if (record.owned_widget ||
            !std::holds_alternative<std::monostate>(record.owned_configuration))
            throw std::logic_error("UI terminal storage cleanup requires prior destroy_bindings");
    for (auto& record : records)
        std::vector<int>{}.swap(record.callback_references);
    std::vector<UiBinding>{}.swap(records);
    // Context objects were destroyed by destroy_bindings; release only the
    //now-empty source support allocation, without invoking widget callbacks.
    std::vector<std::unique_ptr<CallbackContext>>{}.swap(callback_contexts_);
}

} // namespace skeet::script
