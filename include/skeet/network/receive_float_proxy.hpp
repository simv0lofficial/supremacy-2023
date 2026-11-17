#pragma once

namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::network {
using ReceiveFloatProxyFunction = void(__cdecl*)(void*);

// Complete ordinary43389E01..4338A0A8. The SDK argument carries its
// descriptor at+30 and bit-reader at+34; descriptor bit widths are0..32.
void receive_float_value(void* argument, ReceiveFloatProxyFunction const& original);
// Borrow the canonical live43476A44 original-function cell. Its data owner
// must outlive installation and restoration of engine.dll's float receiver.
void bind_receive_float_proxy(engine::NativeDataImage&) noexcept;
void __cdecl receive_float_proxy(void* argument);
} //namespace skeet::network
