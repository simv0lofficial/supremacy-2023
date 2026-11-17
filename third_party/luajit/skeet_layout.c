/* Source integration metadata, compiled with the same ABI as the LuaJIT core. */
#include "lj_dispatch.h"
#include <stddef.h>

#if !LJ_TARGET_X86 || LJ_64 || LJ_GC64 || LJ_FR2 || !LJ_52 || !LJ_HASJIT || !LJ_HASFFI
#error The client requires x86 FR1 LuaJIT with Lua52 compatibility, JIT and FFI.
#endif

void skeet_lua_runtime_layout(unsigned int output[3])
{
  output[0] = (unsigned int)offsetof(lua_State, glref);
  output[1] = (unsigned int)sizeof(((lua_State *)0)->glref);
  output[2] = (unsigned int)(offsetof(GG_State, J) + offsetof(jit_State, param)
    + JIT_P_maxmcode * sizeof(int32_t) - offsetof(GG_State, g));
}
