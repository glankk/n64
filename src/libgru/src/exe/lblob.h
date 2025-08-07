#ifndef GRU_LBLOB_H
#define GRU_LBLOB_H
#include "lua.h"
#include "../lib/libgru.h"

void              lgru_blob_register(lua_State *L);
struct gru_blob  *lgru_blob_create(lua_State *L);

#endif
