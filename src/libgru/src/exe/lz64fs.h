#ifndef GRU_LZ64FS_H
#define GRU_LZ64FS_H
#include <lua.h>
#include "../lib/libgru.h"

void              lgru_z64fs_register(lua_State *L);
struct gru_z64fs *lgru_z64fs_create(lua_State *L);

#endif
