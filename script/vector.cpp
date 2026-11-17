#include "skeet/script/vector.hpp"
#include "detail.hpp"
#include <stdexcept>

namespace skeet::script {
namespace {
void* (*closure_context)(void*, int) = nullptr;
constexpr char vector_source[] = R"lua(
local error, type, tonumber = error, type, tonumber
local sin, cos, atan2, sqrt = math.sin, math.cos, math.atan2, math.sqrt
local rad, deg, format = math.rad, math.deg, string.format
local ffi = require("ffi")
local sizeof, istype = ffi.sizeof, ffi.istype
ffi.cdef("typedef struct {\n\tfloat x, y, z;\n} Vector;\n")
local vector_type = ffi.typeof("Vector")
local world_up
local infinity, epsilon = 1/0, 1.1920929e-7
local function vector(x, y, z)
    return vector_type(tonumber(x) or 0, tonumber(y) or 0, tonumber(z) or 0)
end
local function is_vector(value)
    local result=value ~= nil and istype(vector_type,value)
    return result
end
local function is_number(value)
    local result=value ~= nil and type(value)=="number"
    return result
end
local function argument_error(index, expected)
    return error(format("bad argument #%d (expected %s)", index, expected), 4)
end
local function check_vector(value, index)
    local result=is_vector(value) or argument_error(index,"Vector")
    return result
end
local function check_number(value, index)
    local result=is_number(value) or argument_error(index,"number")
    return result
end
local function clone(self) return vector(self.x, self.y, self.z) end
local function unpack_vector(self) return self.x, self.y, self.z end
local function init(self, x, y, z) self.x=x or 0; self.y=y or 0; self.z=z or 0 end
local function init_from_angles(self, pitch, yaw)
    check_number(pitch, 2); check_number(yaw, 3)
    local p, y = rad(pitch), rad(yaw)
    local sp, cp, sy, cy = sin(p), cos(p), sin(y), cos(y)
    self.x=cp*cy; self.y=cp*sy; self.z=-sp
    return self
end
local function square(value) return value*value end
local function multiply_vectors(self, other)
    check_vector(other, 2)
    return vector(self.x*other.x, self.y*other.y, self.z*other.z)
end
local function length2dsqr(self) return square(self.x)+square(self.y) end
local function lengthsqr(self) return (square(self.x)+square(self.y))+square(self.z) end
local function length2d(self) return sqrt(length2dsqr(self)) end
local function length(self) return sqrt(lengthsqr(self)) end
local function dot(self, other)
    check_vector(other, 2)
    return (self.x*other.x+self.y*other.y)+self.z*other.z
end
local function distsqr(self, other)
    check_vector(other, 2)
    return (square(self.x-other.x)+square(self.y-other.y))+square(self.z-other.z)
end
local function dist(self, other)
    check_vector(other, 2)
    return sqrt((square(self.x-other.x)+square(self.y-other.y))+square(self.z-other.z))
end
local function dist2dsqr(self, other)
    check_vector(other, 2)
    return square(self.x-other.x)+square(self.y-other.y)
end
local function dist2d(self, other)
    check_vector(other, 2)
    return sqrt(square(self.x-other.x)+square(self.y-other.y))
end
local function scale(self, value)
    check_number(value, 2)
    self.x=self.x*value; self.y=self.y*value; self.z=self.z*value
end
local function scaled(self, value)
    check_number(value, 2)
    return vector(self.x*value, self.y*value, self.z*value)
end
local function normalize(self)
    local size=length(self)+epsilon
    scale(self, 1/size)
    return size
end
local function normalized(self) return scaled(self, 1/(length(self)+epsilon)) end
local function lerp(self, other, fraction)
    local result=check_vector(other,2) and vector(self.x+(other.x-self.x)*fraction,
        self.y+(other.y-self.y)*fraction,self.z+(other.z-self.z)*fraction)
    return result
end
local function cross(self, other)
    check_vector(other, 2)
    return vector(self.y*other.z-self.z*other.y, self.z*other.x-self.x*other.z,
        self.x*other.y-self.y*other.x)
end
local function angles(self)
    if self.x==0 and self.y==0 then
        return self.z>0 and -90 or 90, 0, 0
    end
    return deg(atan2(-self.z,length2d(self))), deg(atan2(self.y,self.x)), 0
end
local function vectors(self)
    local right, up
    if self.x==0 and self.y==0 then
        right=vector(0,-1,0); up=vector(-self.z,0,0)
    else
        right=cross(self,world_up); normalize(right)
        up=cross(right,self); normalize(up)
    end
    return right, up
end
local function to(self, other)
    check_vector(other, 2)
    local value=vector(other.x-self.x,other.y-self.y,other.z-self.z)
    normalize(value)
    return value
end
local function equal(self, other)
    return is_vector(other) and self.x==other.x and self.y==other.y and self.z==other.z
end
local function negate(self) return vector(-self.x,-self.y,-self.z) end
local function add_vectors(self, other)
    local result=check_vector(other,3) and vector(self.x+other.x,self.y+other.y,self.z+other.z)
    return result
end
local function add(a,b)
    local result=is_vector(a) and add_vectors(a,b) or add_vectors(b,a)
    return result
end
local function subtract_vectors(self, other)
    local result=check_vector(other,2) and vector(self.x-other.x,self.y-other.y,self.z-other.z)
    return result
end
local function subtract(a,b)
    local result=is_vector(a) and subtract_vectors(a,b) or subtract_vectors(b,a)
    return result
end
local function multiply_value(self, other)
    local result=is_vector(other) and multiply_vectors(self,other) or scaled(self,other)
    return result
end
local function multiply(a,b)
    local result=is_vector(a) and multiply_value(a,b) or multiply_value(b,a)
    return result
end
local function divide_value(self,value)
    local result=check_number(value) and scaled(self,1/value)
    return result
end
local function divide(a,b)
    local result=is_vector(a) and divide_value(a,b) or divide_value(b,a)
    return result
end
local function text(self) return format("%f %f %f",self.x,self.y,self.z) end
local metatable={}
metatable.__add=add; metatable.__div=divide; metatable.__eq=equal; metatable.__len=length
metatable.__mul=multiply; metatable.__sub=subtract; metatable.__tostring=text; metatable.__unm=negate
local methods={}
methods.clone=clone; methods.unpack=unpack_vector; methods.init=init; methods.init_from_angles=init_from_angles
methods.scale=scale; methods.scaled=scaled; methods.normalize=normalize; methods.normalized=normalized
methods.length2d=length2d; methods.length=length; methods.length2dsqr=length2dsqr; methods.lengthsqr=lengthsqr
methods.lerp=lerp; methods.dot=dot; methods.dist=dist; methods.distsqr=distsqr
methods.dist2d=dist2d; methods.dist2dsqr=dist2dsqr; methods.cross=cross; methods.angles=angles
methods.vectors=vectors; methods.to=to
metatable.__index=methods
vector_type=ffi.metatype(vector_type,metatable)
world_up=vector_type(0,0,1)
return vector
)lua";
}

std::string_view VectorModule::source() noexcept {
    return vector_source;
}

int VectorModule::entry(void* state) {
    return static_cast<VectorModule*>(closure_context(state, -10003))->open(state);
}

void VectorModule::preload() {
    const auto& api = *bridge_.api;
    if (!bridge_.state)
        throw std::logic_error("Vector preload requires an initialized Lua state");
    closure_context = api.touserdata;
    api_.findtable(bridge_.state, -10000, "_PRELOAD", 1);
    api.pushlightuserdata(bridge_.state, this);
    api.pushcclosure(bridge_.state, entry, 1);
    api.setfield(bridge_.state, -2, "vector");
    api.settop(bridge_.state, -2);
}

int VectorModule::open() {
    return open(bridge_.state);
}

int VectorModule::open(void* state) {
    detail::CallbackFrame frame{};
    frame.active.flags = 0x67736c6a;
    loading_builtin_ = true;
    detail::RtlPushFrame(&frame.active);
    auto load = [&] {
        const auto& api = *bridge_.api;
        if (api_.loadbuffer(state, vector_source, sizeof(vector_source) - 1, "vector") != 0 ||
            api.pcall(state, 0, 1, 0) != 0)
            detail::report_lua_error(bridge_, state);
    };
    detail::protected_region(load, frame);
    detail::RtlPopFrame(&frame.active);
    loading_builtin_ = false;
    return 1;
}

} // namespace skeet::script
