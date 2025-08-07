#include <config.h>
#include "lua.h"
#include "lauxlib.h"
#include "lgru.h"
#include "../lib/libgru.h"

int lgru_os_rm(lua_State *L)
{
  const char *path = luaL_checkstring(L, 1);
  return lgru_handle_error(L, gru_os_rm(path));
}
