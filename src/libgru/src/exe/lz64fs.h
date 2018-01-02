#ifndef GRU_LZ64FS_H
#define GRU_LZ64FS_H
#include <config.h>
#ifdef HAVE_LUA5_3_LUA_H
#include <lua5.3/lua.h>
#else
#include <lua.h>
#endif
#include "../lib/libgru.h"

void              lgru_z64fs_register(lua_State *L);
struct gru_z64fs *lgru_z64fs_create(lua_State *L);

#endif
