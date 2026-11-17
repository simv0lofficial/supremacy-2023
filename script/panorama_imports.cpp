#include "skeet/script/panorama_api.hpp"
#include "skeet/script/v8_result_abi.hpp"
#include <stdexcept>
#include <string>
#include <type_traits>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace skeet::script {
extern "C" __declspec(dllimport) unsigned char __stdcall RtlFreeHeap(void*, unsigned long, void*);
PanoramaV8Api bind_loaded_panorama_v8_api() {
    const auto module = GetModuleHandleW(L"v8.dll");
    if (!module)
        throw std::runtime_error("Panorama requires loaded v8.dll");
    PanoramaV8Api api{};
    auto bind = [&](auto& function, const char* symbol) {
        const auto address = GetProcAddress(module, symbol);
        if (!address)
            throw std::runtime_error(std::string("Missing V8 export: ") + symbol);
        function = reinterpret_cast<std::remove_reference_t<decltype(function)>>(address);
    };
    auto result_address = [&](const char* symbol) -> void* {
        const auto address = GetProcAddress(module, symbol);
        if (!address)
            throw std::runtime_error(std::string("Missing V8 export: ") + symbol);
        return reinterpret_cast<void*>(address);
    };
    auto bind_static_result = [&]<auto Member>(const char* symbol) {
        api.*Member = v8_abi::bind_static<Member>(result_address(symbol));
    };
    auto bind_method_result = [&]<auto Member>(const char* symbol) {
        api.*Member = v8_abi::bind_method<Member>(result_address(symbol));
    };
    bind(api.isolate_enter, "?Enter@Isolate@v8@@QAEXXZ");
    bind(api.isolate_exit, "?Exit@Isolate@v8@@QAEXXZ");
    bind(api.handle_scope_construct, "??0HandleScope@v8@@QAE@PAVIsolate@1@@Z");
    bind(api.handle_scope_destroy, "??1HandleScope@v8@@QAE@XZ");
    bind(api.try_catch_construct, "??0TryCatch@v8@@QAE@PAVIsolate@1@@Z");
    bind(api.try_catch_destroy, "??1TryCatch@v8@@QAE@XZ");
    bind(api.has_caught, "?HasCaught@TryCatch@v8@@QBE_NXZ");
    bind_method_result.operator()<&PanoramaV8Api::exception>(
        "?Exception@TryCatch@v8@@QBE?AV?$Local@VValue@v8@@@2@XZ");
    bind(api.create_handle,
         "?CreateHandle@HandleScope@v8@@KAPAPAVObject@internal@2@PAVIsolate@42@PAV342@@Z");
    bind(api.globalize_reference,
         "?GlobalizeReference@V8@v8@@CAPAPAVObject@internal@2@PAVIsolate@42@PAPAV342@@Z");
    bind(api.dispose_global, "?DisposeGlobal@V8@v8@@CAXPAPAVObject@internal@2@@Z");
    bind(api.context_enter, "?Enter@Context@v8@@QAEXXZ");
    bind(api.context_exit, "?Exit@Context@v8@@QAEXXZ");
    bind_method_result.operator()<&PanoramaV8Api::context_global>(
        "?Global@Context@v8@@QAE?AV?$Local@VObject@v8@@@2@XZ");
    bind_static_result.operator()<&PanoramaV8Api::string_new>(
        "?NewFromUtf8@String@v8@@SA?AV?$Local@VString@v8@@@2@PAVIsolate@2@PBDW4NewStringType@12@H@Z");
    bind_static_result.operator()<&PanoramaV8Api::string_maybe_new>(
        "?NewFromUtf8@String@v8@@SA?AV?$MaybeLocal@VString@v8@@@2@PAVIsolate@2@PBDW4NewStringType@2@H@Z");
    bind_static_result.operator()<&PanoramaV8Api::number_new>(
        "?New@Number@v8@@SA?AV?$Local@VNumber@v8@@@2@PAVIsolate@2@N@Z");
    bind_static_result.operator()<&PanoramaV8Api::object_new>(
        "?New@Object@v8@@SA?AV?$Local@VObject@v8@@@2@PAVIsolate@2@@Z");
    bind_static_result.operator()<&PanoramaV8Api::array_new>(
        "?New@Array@v8@@SA?AV?$Local@VArray@v8@@@2@PAVIsolate@2@H@Z");
    bind_static_result.operator()<&PanoramaV8Api::compile_unbound_script>(
        "?CompileUnboundScript@ScriptCompiler@v8@@SA?AV?$MaybeLocal@VUnboundScript@v8@@@2@PAVIsolate@2@PAVSource@12@W4CompileOptions@12@@Z");
    bind_method_result.operator()<&PanoramaV8Api::bind_to_current_context>(
        "?BindToCurrentContext@UnboundScript@v8@@QAE?AV?$Local@VScript@v8@@@2@XZ");
    bind_method_result.operator()<&PanoramaV8Api::script_run>(
        "?Run@Script@v8@@QAE?AV?$MaybeLocal@VValue@v8@@@2@V?$Local@VContext@v8@@@2@@Z");
    bind_method_result.operator()<&PanoramaV8Api::object_get>(
        "?Get@Object@v8@@QAE?AV?$MaybeLocal@VValue@v8@@@2@V?$Local@VContext@v8@@@2@V?$Local@VValue@v8@@@2@@Z");
    bind(api.object_set, "?Set@Object@v8@@QAE_NV?$Local@VValue@v8@@@2@0@Z");
    bind_method_result.operator()<&PanoramaV8Api::array_set>(
        "?Set@Object@v8@@QAE?AV?$Maybe@_N@2@V?$Local@VContext@v8@@@2@IV?$Local@VValue@v8@@@2@@Z");
    bind_method_result.operator()<&PanoramaV8Api::array_get>(
        "?Get@Object@v8@@QAE?AV?$MaybeLocal@VValue@v8@@@2@V?$Local@VContext@v8@@@2@I@Z");
    bind(api.array_length, "?Length@Array@v8@@QBEIXZ");
    bind_method_result.operator()<&PanoramaV8Api::function_call>(
        "?Call@Function@v8@@QAE?AV?$MaybeLocal@VValue@v8@@@2@V?$Local@VContext@v8@@@2@V?$Local@VValue@v8@@@2@HQAV52@@Z");
    bind_method_result.operator()<&PanoramaV8Api::object_call>(
        "?CallAsFunction@Object@v8@@QAE?AV?$MaybeLocal@VValue@v8@@@2@V?$Local@VContext@v8@@@2@V?$Local@VValue@v8@@@2@HQAV52@@Z");
    bind(api.is_function, "?IsFunction@Value@v8@@QBE_NXZ");
    bind(api.is_object, "?IsObject@Value@v8@@QBE_NXZ");
    bind(api.is_array, "?IsArray@Value@v8@@QBE_NXZ");
    bind(api.is_callable, "?IsCallable@Object@v8@@QAE_NXZ");
    bind(api.is_boolean, "?IsBoolean@Value@v8@@QBE_NXZ");
    bind(api.boolean_value, "?IsTrue@Value@v8@@QBE_NXZ");
    bind(api.is_int32, "?IsInt32@Value@v8@@QBE_NXZ");
    bind(api.is_uint32, "?IsUint32@Value@v8@@QBE_NXZ");
    bind(api.is_number, "?IsNumber@Value@v8@@QBE_NXZ");
    bind_method_result.operator()<&PanoramaV8Api::to_int32>(
        "?ToInt32@Value@v8@@QBE?AV?$MaybeLocal@VInt32@v8@@@2@V?$Local@VContext@v8@@@2@@Z");
    bind_method_result.operator()<&PanoramaV8Api::to_uint32>(
        "?ToUint32@Value@v8@@QBE?AV?$MaybeLocal@VUint32@v8@@@2@V?$Local@VContext@v8@@@2@@Z");
    bind_method_result.operator()<&PanoramaV8Api::to_number>(
        "?ToNumber@Value@v8@@QBE?AV?$MaybeLocal@VNumber@v8@@@2@V?$Local@VContext@v8@@@2@@Z");
    bind_method_result.operator()<&PanoramaV8Api::to_object>(
        "?ToObject@Value@v8@@QBE?AV?$MaybeLocal@VObject@v8@@@2@V?$Local@VContext@v8@@@2@@Z");
    bind_method_result.operator()<&PanoramaV8Api::to_string>(
        "?ToString@Value@v8@@QBE?AV?$MaybeLocal@VString@v8@@@2@V?$Local@VContext@v8@@@2@@Z");
    bind(api.int32_value, "?Value@Int32@v8@@QBEHXZ");
    bind(api.uint32_value, "?Value@Uint32@v8@@QBEIXZ");
    bind(api.number_value, "?Value@Number@v8@@QBENXZ");
    bind(api.utf8_construct, "??0Utf8Value@String@v8@@QAE@V?$Local@VValue@v8@@@2@@Z");
    bind(api.utf8_destroy, "??1Utf8Value@String@v8@@QAE@XZ");
    bind(api.cached_data_destroy, "??1CachedData@ScriptCompiler@v8@@QAE@XZ");
    api.process_heap = GetProcessHeap();
    api.heap_free = RtlFreeHeap;
    return api;
}
} // namespace skeet::script
