#ifndef GRU_LUPS_H
#define GRU_LUPS_H
#include "lua.h"
#include "../lib/libgru.h"

void            lgru_ups_register(lua_State *L);
struct gru_ups *lgru_ups_create(lua_State *L);

#endif
