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

static int lgru_ups_load(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  const char *filename = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_ups_load(ups, filename));
}

static int lgru_ups_save(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  const char *filename = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_ups_save(ups, filename));
}

static int lgru_ups_make(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  struct gru_blob *src_blob = lgru_checkclass(L, 2, "gru_blob");
  struct gru_blob *dst_blob = lgru_checkclass(L, 2, "gru_blob");
  return lgru_handle_error(L, gru_ups_make(ups, src_blob, dst_blob));
}

static int lgru_ups_apply(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  struct gru_blob *blob = lgru_checkclass(L, 2, "gru_blob");
  gru_bool_t ignore_checksums = lua_toboolean(L, 3);
  return lgru_handle_error(L, gru_ups_apply(ups, blob, ignore_checksums));
}

static int lgru_ups_undo(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  struct gru_blob *blob = lgru_checkclass(L, 2, "gru_blob");
  gru_bool_t ignore_checksums = lua_toboolean(L, 3);
  return lgru_handle_error(L, gru_ups_undo(ups, blob, ignore_checksums));
}

static int lgru_ups_src_size(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  lua_pushinteger(L, gru_ups_src_size(ups));
  return 1;
}

static int lgru_ups_dst_size(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  lua_pushinteger(L, gru_ups_dst_size(ups));
  return 1;
}

static int lgru_ups_src_crc(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  lua_pushinteger(L, gru_ups_src_crc(ups));
  return 1;
}

static int lgru_ups_dst_crc(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  lua_pushinteger(L, gru_ups_dst_crc(ups));
  return 1;
}

static int lgru_ups_gc(lua_State *L)
{
  struct gru_ups *ups = lgru_checkclass(L, 1, "gru_ups");
  gru_ups_destroy(ups);
  return 0;
}

void lgru_ups_register(lua_State *L)
{
  if (luaL_newmetatable(L, "gru_ups")) {
    /* methods */
    lua_pushcfunction(L, lgru_ups_load);
    lua_setfield(L, -2, "load");
    lua_pushcfunction(L, lgru_ups_save);
    lua_setfield(L, -2, "save");
    lua_pushcfunction(L, lgru_ups_make);
    lua_setfield(L, -2, "make");
    lua_pushcfunction(L, lgru_ups_apply);
    lua_setfield(L, -2, "apply");
    lua_pushcfunction(L, lgru_ups_undo);
    lua_setfield(L, -2, "undo");
    lua_pushcfunction(L, lgru_ups_src_size);
    lua_setfield(L, -2, "src_size");
    lua_pushcfunction(L, lgru_ups_dst_size);
    lua_setfield(L, -2, "dst_size");
    lua_pushcfunction(L, lgru_ups_src_crc);
    lua_setfield(L, -2, "src_crc");
    lua_pushcfunction(L, lgru_ups_dst_crc);
    lua_setfield(L, -2, "dst_crc");
    /* meta */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lgru_ups_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
  }
}

struct gru_ups *lgru_ups_create(lua_State *L)
{
  struct gru_ups *ups = lua_newuserdata(L, sizeof(struct gru_ups));
  gru_ups_init(ups);
  lgru_ups_register(L);
  lua_setmetatable(L, -2);
  return ups;
}
