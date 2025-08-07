#ifndef GRU_LGSC_H
#define GRU_LGSC_H
#include "lua.h"
#include "../lib/libgru.h"

void            lgru_gsc_register(lua_State *L);
struct gru_gsc *lgru_gsc_create(lua_State *L);

#endif
