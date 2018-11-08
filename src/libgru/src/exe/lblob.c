#include <config.h>
#include <stdlib.h>
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
#include "lblob.h"
#include "../lib/libgru.h"

static int lgru_blob_load(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  const char *filename = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_blob_load(blob, filename));
}

static int lgru_blob_save(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  const char *filename = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_blob_save(blob, filename));
}

static int lgru_blob_size(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  lua_pushinteger(L, gru_blob_size(blob));
  return 1;
}

static int lgru_blob_resize(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t size = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_blob_resize(blob, size));
}

static int lgru_blob_ensure_size(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t size = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_blob_ensure_size(blob, size));
}

static int lgru_blob_crc32(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t start = luaL_optinteger(L, 2, 0);
  size_t size = luaL_optinteger(L, 3, blob->size - start);
  uint32_t crc32;
  enum gru_error e = gru_blob_crc32(blob, start, size, &crc32);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, crc32);
  return 1;
}

static int lgru_blob_write(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  struct gru_blob *src_blob = lgru_checkclass(L, 3, "gru_blob");
  size_t src_address = luaL_optinteger(L, 4, 0);
  size_t size = luaL_optinteger(L, 5, src_blob->size - src_address);
  return lgru_handle_error(L, gru_blob_write(blob, address,
                                             src_blob, src_address, size));
}

static int lgru_blob_copy(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t start = luaL_optinteger(L, 2, 0);
  size_t size = luaL_optinteger(L, 3, blob->size - start);
  struct gru_blob *blob_copy = lgru_blob_create(L);
  enum gru_error e = gru_blob_copy(blob, start, size, blob_copy);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_blob_write8(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint8_t value = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_blob_write8(blob, address, value));
}

static int lgru_blob_read8(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint8_t value;
  enum gru_error e = gru_blob_read8(blob, address, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, value);
  return 1;
}

static int lgru_blob_write16be(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint16_t value = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_blob_write16be(blob, address, value));
}

static int lgru_blob_write16le(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint16_t value = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_blob_write16le(blob, address, value));
}

static int lgru_blob_read16be(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint16_t value;
  enum gru_error e = gru_blob_read16be(blob, address, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, value);
  return 1;
}

static int lgru_blob_read16le(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint16_t value;
  enum gru_error e = gru_blob_read16le(blob, address, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, value);
  return 1;
}

static int lgru_blob_write32be(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint32_t value = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_blob_write32be(blob, address, value));
}

static int lgru_blob_write32le(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint32_t value = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_blob_write32le(blob, address, value));
}

static int lgru_blob_read32be(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint32_t value;
  enum gru_error e = gru_blob_read32be(blob, address, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, value);
  return 1;
}

static int lgru_blob_read32le(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  uint32_t value;
  enum gru_error e = gru_blob_read32le(blob, address, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, value);
  return 1;
}

static int lgru_blob_writefloatbe(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  float value = luaL_checknumber(L, 3);
  return lgru_handle_error(L, gru_blob_writefloatbe(blob, address, value));
}

static int lgru_blob_writefloatle(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  float value = luaL_checknumber(L, 3);
  return lgru_handle_error(L, gru_blob_writefloatle(blob, address, value));
}

static int lgru_blob_readfloatbe(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  float value;
  enum gru_error e = gru_blob_readfloatbe(blob, address, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushnumber(L, value);
  return 1;
}

static int lgru_blob_readfloatle(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  float value;
  enum gru_error e = gru_blob_readfloatle(blob, address, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushnumber(L, value);
  return 1;
}

static int lgru_blob_writestring(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_checkinteger(L, 2);
  size_t size;
  const char *value = luaL_tolstring(L, 3, &size);
  return lgru_handle_error(L, gru_blob_writestring(blob, address, size, value));
}

static int lgru_blob_readstring(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t address = luaL_optinteger(L, 2, 0);
  size_t size = luaL_optinteger(L, 3, blob->size);
  char *value;
  enum gru_error e = gru_blob_readstring(blob, address, size, &value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushlstring(L, value, size);
  free(value);
  return 1;
}

static int lgru_blob_swap(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t word_size = luaL_checkinteger(L, 2);
  gru_blob_swap(blob, word_size);
  return 0;
}

static int lgru_blob_find(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  struct gru_blob *search = lgru_checkclass(L, 2, "gru_blob");
  size_t start = luaL_optinteger(L, 3, 0);
  size_t length = luaL_optinteger(L, 4, 0);
  size_t pos = gru_blob_find(blob, search, start, length);
  if (pos == -1)
    lua_pushnil(L);
  else
    lua_pushinteger(L, pos);
  return 1;
}

static int lgru_blob_gc(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  gru_blob_destroy(blob);
  return 0;
}

void lgru_blob_register(lua_State *L)
{
  if (luaL_newmetatable(L, "gru_blob")) {
    /* methods */
    lua_pushcfunction(L, lgru_blob_load);
    lua_setfield(L, -2, "load");
    lua_pushcfunction(L, lgru_blob_save);
    lua_setfield(L, -2, "save");
    lua_pushcfunction(L, lgru_blob_size);
    lua_setfield(L, -2, "size");
    lua_pushcfunction(L, lgru_blob_ensure_size);
    lua_setfield(L, -2, "ensure_size");
    lua_pushcfunction(L, lgru_blob_resize);
    lua_setfield(L, -2, "resize");
    lua_pushcfunction(L, lgru_blob_crc32);
    lua_setfield(L, -2, "crc32");
    lua_pushcfunction(L, lgru_blob_write);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, lgru_blob_copy);
    lua_setfield(L, -2, "copy");
    lua_pushcfunction(L, lgru_blob_write8);
    lua_setfield(L, -2, "write8");
    lua_pushcfunction(L, lgru_blob_read8);
    lua_setfield(L, -2, "read8");
    lua_pushcfunction(L, lgru_blob_write16be);
    lua_setfield(L, -2, "write16be");
    lua_pushcfunction(L, lgru_blob_write16le);
    lua_setfield(L, -2, "write16le");
    lua_pushcfunction(L, lgru_blob_read16be);
    lua_setfield(L, -2, "read16be");
    lua_pushcfunction(L, lgru_blob_read16le);
    lua_setfield(L, -2, "read16le");
    lua_pushcfunction(L, lgru_blob_write32be);
    lua_setfield(L, -2, "write32be");
    lua_pushcfunction(L, lgru_blob_write32le);
    lua_setfield(L, -2, "write32le");
    lua_pushcfunction(L, lgru_blob_read32be);
    lua_setfield(L, -2, "read32be");
    lua_pushcfunction(L, lgru_blob_read32le);
    lua_setfield(L, -2, "read32le");
    lua_pushcfunction(L, lgru_blob_writefloatbe);
    lua_setfield(L, -2, "writefloatbe");
    lua_pushcfunction(L, lgru_blob_writefloatle);
    lua_setfield(L, -2, "writefloatle");
    lua_pushcfunction(L, lgru_blob_readfloatbe);
    lua_setfield(L, -2, "readfloatbe");
    lua_pushcfunction(L, lgru_blob_readfloatle);
    lua_setfield(L, -2, "readfloatle");
    lua_pushcfunction(L, lgru_blob_writestring);
    lua_setfield(L, -2, "writestring");
    lua_pushcfunction(L, lgru_blob_readstring);
    lua_setfield(L, -2, "readstring");
    lua_pushcfunction(L, lgru_blob_swap);
    lua_setfield(L, -2, "swap");
    lua_pushcfunction(L, lgru_blob_find);
    lua_setfield(L, -2, "find");
    /* meta */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lgru_blob_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
  }
}

struct gru_blob *lgru_blob_create(lua_State *L)
{
  struct gru_blob *blob = lua_newuserdata(L, sizeof(struct gru_blob));
  gru_blob_init(blob);
  lgru_blob_register(L);
  lua_setmetatable(L, -2);
  return blob;
}
