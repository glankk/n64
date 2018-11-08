#include <config.h>
#if defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lua.h>
#include <lua53/lauxlib.h>
#elif defined(HAVE_LUA_H)
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
