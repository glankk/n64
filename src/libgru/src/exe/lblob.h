#ifndef GRU_LBLOB_H
#define GRU_LBLOB_H
#include <config.h>
#ifdef HAVE_LUA5_3_LUA_H
#include <lua5.3/lua.h>
#else
#include <lua.h>
#endif
#include "../lib/libgru.h"

void              lgru_blob_register(lua_State *L);
struct gru_blob  *lgru_blob_create(lua_State *L);

#endif
