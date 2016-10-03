#ifndef GRU_LN64ROM_H
#define GRU_LN64ROM_H
#include <lua.h>
#include "../lib/libgru.h"

void                lgru_n64rom_register(lua_State *L);
struct gru_n64rom  *lgru_n64rom_create(lua_State *L);

#endif
