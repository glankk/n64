#include <config.h>
#include "lua.h"
#include "lauxlib.h"
#include "lgru.h"
#include "../lib/libgru.h"

static int lgru_gsc_load(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  const char *filename = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_gsc_load(gsc, filename));
}

static int lgru_gsc_save(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  const char *filename = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_gsc_save(gsc, filename));
}

static int lgru_gsc_size(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  lua_pushinteger(L, gru_gsc_size(gsc));
  return 1;
}

static int lgru_gsc_insert(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  size_t index = luaL_checkinteger(L, 2);
  uint32_t address = luaL_checkinteger(L, 3);
  uint16_t value = luaL_checkinteger(L, 4);
  return lgru_handle_error(L, gru_gsc_insert(gsc, index, address, value));
}

static int lgru_gsc_replace(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  size_t index = luaL_checkinteger(L, 2);
  uint32_t address = luaL_checkinteger(L, 3);
  uint16_t value = luaL_checkinteger(L, 4);
  return lgru_handle_error(L, gru_gsc_replace(gsc, index, address, value));
}

static int lgru_gsc_remove(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  size_t index = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_gsc_remove(gsc, index));
}

static int lgru_gsc_get(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  size_t index = luaL_checkinteger(L, 2);
  uint32_t address;
  uint16_t value;
  lgru_handle_error(L, gru_gsc_get(gsc, index, &address, &value));
  lua_pushinteger(L, address);
  lua_pushinteger(L, value);
  return 2;
}

static int lgru_gsc_shift(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  int32_t offset = luaL_checkinteger(L, 2);
  gru_gsc_shift(gsc, offset);
  return 0;
}

static int lgru_gsc_apply_be(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  struct gru_blob *blob = lgru_checkclass(L, 2, "gru_blob");
  return lgru_handle_error(L, gru_gsc_apply_be(gsc, blob));
}

static int lgru_gsc_apply_le(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  struct gru_blob *blob = lgru_checkclass(L, 2, "gru_blob");
  return lgru_handle_error(L, gru_gsc_apply_le(gsc, blob));
}

static int lgru_gsc_gc(lua_State *L)
{
  struct gru_gsc *gsc = lgru_checkclass(L, 1, "gru_gsc");
  gru_gsc_destroy(gsc);
  return 0;
}

void lgru_gsc_register(lua_State *L)
{
  if (luaL_newmetatable(L, "gru_gsc")) {
    /* methods */
    lua_pushcfunction(L, lgru_gsc_load);
    lua_setfield(L, -2, "load");
    lua_pushcfunction(L, lgru_gsc_save);
    lua_setfield(L, -2, "save");
    lua_pushcfunction(L, lgru_gsc_size);
    lua_setfield(L, -2, "size");
    lua_pushcfunction(L, lgru_gsc_insert);
    lua_setfield(L, -2, "insert");
    lua_pushcfunction(L, lgru_gsc_replace);
    lua_setfield(L, -2, "replace");
    lua_pushcfunction(L, lgru_gsc_remove);
    lua_setfield(L, -2, "remove");
    lua_pushcfunction(L, lgru_gsc_get);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, lgru_gsc_shift);
    lua_setfield(L, -2, "shift");
    lua_pushcfunction(L, lgru_gsc_apply_be);
    lua_setfield(L, -2, "apply_be");
    lua_pushcfunction(L, lgru_gsc_apply_le);
    lua_setfield(L, -2, "apply_le");
    /* meta */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lgru_gsc_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
  }
}

struct gru_gsc *lgru_gsc_create(lua_State *L)
{
  struct gru_gsc *gsc = lua_newuserdata(L, sizeof(struct gru_gsc));
  gru_gsc_init(gsc);
  lgru_gsc_register(L);
  lua_setmetatable(L, -2);
  return gsc;
}
