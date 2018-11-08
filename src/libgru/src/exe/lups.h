#ifndef GRU_LUPS_H
#define GRU_LUPS_H
#include <config.h>
#if defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lua.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lua.h>
#elif defined(HAVE_LUA_H)
#include <lua.h>
#endif
#include "../lib/libgru.h"

void            lgru_ups_register(lua_State *L);
struct gru_ups *lgru_ups_create(lua_State *L);

#endif
