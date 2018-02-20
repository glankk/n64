#include <config.h>
#ifdef HAVE_LUA5_3_LUA_H
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#else
#include <lua.h>
#include <lauxlib.h>
#endif
#include "lgru.h"
#include "../lib/libgru.h"

int lgru_os_rm(lua_State *L)
{
  const char *path = luaL_checkstring(L, 1);
  return lgru_handle_error(L, gru_os_rm(path));
}
