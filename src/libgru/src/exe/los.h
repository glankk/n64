#ifndef GRU_LOS_H
#define GRU_LOS_H
#include <config.h>
#if defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lua.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lua.h>
#elif defined(HAVE_LUA_H)
#include <lua.h>
#endif
#include "../lib/libgru.h"

int lgru_os_rm(lua_State *L);

#endif
