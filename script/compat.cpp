#include "skeet/script/compat.hpp"
#include "detail.hpp"

namespace skeet::script {
namespace {
constexpr char compat_source[] = R"lua(
local error,type,assert,xpcall,pcall=error,type,assert,xpcall,pcall
local setfenv,getfenv,setmetatable,require,rawset,load=setfenv,getfenv,setmetatable,require,rawset,load
local lower,format=string.lower,string.format
local pack,unpack,insert,move=table.pack,table.unpack,table.insert,table.move
local clear=require('table.clear')
local ffi=require('ffi')
local cast,typeof=ffi.cast,ffi.typeof
local client=client
local create_interface=client.create_interface
local renderer,panorama,plist=renderer,panorama,plist
local panorama_loadstring=panorama.loadstring
local realtime,tickinterval=globals.realtime,globals.tickinterval
local error_log=client.error_log
local flush=database.flush
local plist_get,plist_set=plist.get,plist.set
local log=client.log
local floor=math.floor
local PREVIOUS,NEXT,TIME,FUNCTION,ARGUMENTS=1,2,3,4,5
local DEFER_FUNCTION,DEFER_ARGUMENTS=1,2
local vtable_type=typeof('void***')
local command_type=typeof('struct { int chokedcommands,command_number;float pitch,yaw,roll,forwardmove,sidemove;bool allow_send_packet;bool force_defensive;union { uint32_t buttons;struct {uint32_t in_attack:1;uint32_t in_jump:1;uint32_t in_duck:1;uint32_t in_forward:1;uint32_t in_back:1;uint32_t in_use:1;uint32_t in_cancel:1;uint32_t in_left:1;uint32_t in_right:1;uint32_t in_moveleft:1;uint32_t in_moveright:1;uint32_t in_attack2:1;uint32_t in_run:1;uint32_t in_reload:1;uint32_t in_alt1:1;uint32_t in_alt2:1;uint32_t in_score:1;uint32_t in_speed:1;uint32_t in_walk:1;uint32_t in_zoom:1;uint32_t in_weapon1:1;uint32_t in_weapon2:1;uint32_t in_bullrush:1;uint32_t in_grenade1:1;uint32_t in_grenade2:1;uint32_t in_attack3:1;uint32_t in_unused:6;};};int weaponselect,weaponsubtype;float move_yaw;bool no_choke;bool quick_stop;bool allow_shift_tickbase;bool discharge_pending;}*')
local environment_metatable={__index=_G}
local deferred,temporary_deferred,scheduled={},{},{}
local player_values={
    ['override prefer body aim']={[0]='-','Off','On','Force'},
    ['override safe point']={[0]='-','Off','On'},
}
local function panorama_load(code,...)
    return panorama_loadstring('(function (){'..code..'})();',...)
end
local function player_get(player,key)
    local value=plist_get(player,key)
    local values=player_values[lower(key)]
    if values~=nil then value=values[value] end
    return value
end
local function player_set(player,key,value)
    local values=player_values[lower(key)]
    if values~=nil then
        local maximum,size=#values,#value
        for index=0,maximum do
            if #values[index]==size and values[index]==value then value=index;break end
        end
    end
    return plist_set(player,key,value)
end
local function initialize_list(node)
    node[PREVIOUS]=node;node[NEXT]=node
    return node
end
local function list_empty(node)
    local result=node[NEXT]==node
    return result
end
local function insert_before(position,node)
    local previous=position[PREVIOUS]
    node[PREVIOUS]=previous;node[NEXT]=position
    previous[NEXT]=node;position[PREVIOUS]=node
end
local function detach(node)node[NEXT]=nil;node[PREVIOUS]=nil end
local function remove(node)
    local following,previous=node[NEXT],node[PREVIOUS]
    previous[NEXT]=following;following[PREVIOUS]=previous
    detach(node)
    local result=following==previous
    return result
end
local function append_before(position,node)
    local previous=position[PREVIOUS]
    node[NEXT]=position;node[PREVIOUS]=previous
    previous[NEXT]=node;position[PREVIOUS]=node
end
local function delay_call(delay,callback,...)
    if type(delay)~='number' then return error('invalid arg') end
    if type(callback)~='function' then return error('invalid arg') end
    local due=realtime()+delay
    local node={nil,nil,due,callback,pack(...)}
    local position=scheduled[NEXT]
    while position~=scheduled do
        if due<position[TIME] then break end
        position=position[NEXT]
    end
    return insert_before(position,node)
end
local function run_scheduled()
    if list_empty(scheduled) then return end
    local now=realtime()
    local ready=nil
    local node=scheduled[NEXT]
    while node~=scheduled do
        local following=node[NEXT]
        if node[TIME]<=now then
            remove(node)
            if ready==nil then ready=initialize_list({}) end
            append_before(ready,node)
        end
        node=following
    end
    if ready==nil then return end
    node=ready[NEXT]
    while node~=ready do
        xpcall(node[FUNCTION],error_log,unpack(node[ARGUMENTS],1,node[ARGUMENTS].n))
        node=node[NEXT]
    end
end
local function clear_scheduled()initialize_list(scheduled) end
local function add_deferred(list,callback,...)
    if type(callback)~='function' then return error('invalid arg',2) end
    return insert(list,{callback,pack(...)})
end
local function defer(callback,...)return add_deferred(deferred,callback,...)end
local function temporary_defer(callback,...)return add_deferred(temporary_deferred,callback,...)end
local function run_deferred(list)
    for index=#list,1,-1 do
        local entry=list[index]
        if entry[DEFER_FUNCTION]~=nil then
            xpcall(entry[DEFER_FUNCTION],error_log,unpack(entry[DEFER_ARGUMENTS],1,entry[DEFER_ARGUMENTS].n))
        end
    end
    return clear(list)
end
local function shutdown()
    run_deferred(deferred)
    return flush()
end
local function execute(loader,name,source)
    local previous_environment=getfenv(0)
    local environment=setmetatable({_NAME=name},environment_metatable)
    setfenv(0,environment)
    rawset(environment,'defer',temporary_defer)
    assert(loader~=nil)
    local succeeded,result=pcall(loader,name,source,environment)
    rawset(environment,'defer',nil)
    if succeeded then
        move(temporary_deferred,1,#temporary_deferred,#deferred+1,deferred)
        clear(temporary_deferred)
    else
        run_deferred(temporary_deferred)
    end
    setfenv(0,previous_environment)
    if not succeeded then return error(result) end
    return succeeded,result
end
local function require_module(name)return execute(require,name,nil)end
local function execute_source(name,source,environment)
    assert(name~=nil);assert(source~=nil)
    local chunk,message=load(source,name,'bt',environment)
    if chunk==nil then return error(message) end
    return chunk()
end
local function load_module(name,source)return execute(execute_source,name,source)end
local function load_callback(name,source)
    local chunk,message=load(source,'@'..name,'bt')
    if chunk==nil then return error(message) end
    return function()
        local _,result=execute(function(_,_,environment)
            setfenv(chunk,environment)
            return chunk(name)
        end,name,nil)
        return result
    end
end
local function command(pointer)return cast(command_type,pointer)end
local function register_render_aliases()
    local old={'draw_text','draw_rectangle','draw_line','draw_gradient','draw_circle','draw_circle_outline','world_to_screen','draw_indicator'}
    local current={'text','rectangle','line','gradient','circle','circle_outline','world_to_screen','indicator'}
    for index=1,#old do
        local callback=renderer[current[index]]
        client[old[index]]=function(_,...)return callback(...)end
    end
end
local function vtable_function(instance,index,ctype)
    return cast(ctype,cast(vtable_type,instance)[0][index])
end
local function vtable_bind(module,interface,index,ctype,...)
    local instance=create_interface(module,interface) or error('invalid interface',2)
    local callback=vtable_function(instance,index,typeof(ctype,...)) or error('invalid vtable',2)
    return function(...)return callback(instance,...)end
end
local function vtable_thunk(index,ctype,...)
    local method_type=typeof(ctype,...)
    return function(instance,...)
        if instance then return vtable_function(instance,index,method_type)(instance,...) end
        return error('invalid instance',2)
    end
end
local function printf(text,...)return log(format(text,...))end
local function toticks(seconds)return floor(0.5+seconds/tickinterval())end
local function totime(ticks)return ticks*tickinterval()end
initialize_list(scheduled)
register_render_aliases()
rawset(_G,'print',log);rawset(_G,'printf',printf);rawset(_G,'defer',defer)
rawset(_G,'vtable_bind',vtable_bind);rawset(_G,'vtable_thunk',vtable_thunk)
rawset(_G,'toticks',toticks);rawset(_G,'totime',totime)
rawset(panorama,'loadstring',panorama_load);rawset(client,'delay_call',delay_call)
rawset(plist,'get',player_get);rawset(plist,'set',player_set)
return {command,shutdown,require_module,load_module,load_callback,run_scheduled,clear_scheduled}
)lua";
}

std::string_view CompatModule::source() noexcept {
    return compat_source;
}

int CompatModule::open() {
    detail::CallbackFrame frame{};
    frame.active.flags = 0x67736c6a;
    int result = 1;
    loading_builtin_ = true;
    detail::RtlPushFrame(&frame.active);
    auto load = [&] {
        result = loadbuffer_(bridge_.state, compat_source, sizeof(compat_source) - 1, "compat");
        if (result == 0)
            result = bridge_.api->pcall(bridge_.state, 0, 1, 0);
        if (result != 0)
            detail::report_lua_error(bridge_);
    };
    if (!detail::protected_region(load, frame))
        result = 1;
    detail::RtlPopFrame(&frame.active);
    if (result != 0)
        detail::report_native_exception(bridge_, frame, "initializing Lua compatibility module");
    loading_builtin_ = false;
    return result;
}
}
