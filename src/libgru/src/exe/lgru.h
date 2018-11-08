#ifndef GRU_LGRU_H
#define GRU_LGRU_H
#include <config.h>
#if defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lua.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lua.h>
#elif defined(HAVE_LUA_H)
#include <lua.h>
#endif
#include "../lib/libgru.h"

void *lgru_checkclass(lua_State *L, int arg, const char *tname);
int lgru_handle_error(lua_State *L, enum gru_error e);
int lgru_handle_error_noreturn(lua_State *L, enum gru_error e);
void lgru_gru_register(lua_State *L);
void lgru_gru_create(lua_State *L);

#endif
