#ifndef GRU_LN64ROM_H
#define GRU_LN64ROM_H
#include <config.h>
#ifdef HAVE_LUA5_3_LUA_H
#include <lua5.3/lua.h>
#else
#include <lua.h>
#endif
#include "../lib/libgru.h"

void                lgru_n64rom_register(lua_State *L);
struct gru_n64rom  *lgru_n64rom_create(lua_State *L);

#endif
