#ifndef GRU_LOS_H
#define GRU_LOS_H
#include <config.h>
#ifdef HAVE_LUA5_3_LUA_H
#include <lua5.3/lua.h>
#else
#include <lua.h>
#endif
#include "../lib/libgru.h"

int lgru_os_rm(lua_State *L);

#endif
