#include "cjson_internal.hpp"

namespace skeet::script::cjson_detail {
namespace {
CjsonApi bound_api;
}
const CjsonApi& api() {
    return bound_api;
}
void bind_api(const CjsonApi& value) {
    bound_api = value;
}
static_assert(sizeof(void*) != 4 || sizeof(json_config_t) == 0x530);
static_assert(sizeof(void*) != 4 || offsetof(json_config_t, encode_sparse_convert) == 0x510);
}
namespace skeet::script {
int JsonModule::register_exports() {
    // The embedded module also binds one Lua C API shared by its closures.
    // Copying the table retains those imports through Lua's final GC pass.
    cjson_detail::bind_api(api_);
    cjson_detail::fpconv_init();
    cjson_detail::json_create_config(bridge_.state);
    const CjsonRegistration methods[] = {
        {"stringify", cjson_detail::json_encode},
        {"parse", cjson_detail::json_decode},
        {"encode_sparse_array", cjson_detail::json_cfg_encode_sparse_array},
        {"encode_max_depth", cjson_detail::json_cfg_encode_max_depth},
        {"decode_max_depth", cjson_detail::json_cfg_decode_max_depth},
        {"encode_number_precision", cjson_detail::json_cfg_encode_number_precision},
        {"encode_invalid_numbers", cjson_detail::json_cfg_encode_invalid_numbers},
        {"decode_invalid_numbers", cjson_detail::json_cfg_decode_invalid_numbers},
        {nullptr, nullptr}};
    api_.open_library(bridge_.state, "json", methods, 1);
    api_.lua.pushlightuserdata(bridge_.state, nullptr);
    api_.lua.setfield(bridge_.state, -2, "null");
    return 1;
}
} // namespace skeet::script
