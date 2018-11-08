#ifndef GRU_LN64ROM_H
#define GRU_LN64ROM_H
#include <config.h>
#if defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lua.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lua.h>
#elif defined(HAVE_LUA_H)
#include <lua.h>
#endif
#include "../lib/libgru.h"

void                lgru_n64rom_register(lua_State *L);
struct gru_n64rom  *lgru_n64rom_create(lua_State *L);

#endif
