#include <config.h>
#include <string.h>
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
#include "ln64rom.h"
#include "lz64fs.h"
#include "lgsc.h"
#include "los.h"
#include "lups.h"
#include "../lib/libgru.h"

static enum gru_error lgru_last_error   = GRU_SUCCESS;
static gru_bool_t     lgru_fatal_errors = GRU_TRUE;

void *lgru_checkclass(lua_State *L, int arg, const char *tname)
{
  int stack_top = lua_gettop(L);
  if (lua_isuserdata(L, arg) && lua_getmetatable(L, arg)) {
    while (lua_getfield(L, -1, "__name")) {
      const char *s_tname = lua_tostring(L, -1);
      if (s_tname && strcmp(s_tname, tname) == 0) {
        lua_settop(L, stack_top);
        return lua_touserdata(L, arg);
      }
      lua_pop(L, 1);
      if (!luaL_getmetafield(L, -1, "__index"))
        break;
    }
  }
  lua_settop(L, stack_top);
  return luaL_checkudata(L, arg, tname);
}

int lgru_handle_error(lua_State *L, enum gru_error e)
{
  lgru_last_error = e;
  if (e && lgru_fatal_errors)
    return luaL_error(L, "libgru error %d: %s", e, gru_error_string(e));
  lua_pushinteger(L, e);
  return 1;
}

int lgru_handle_error_noreturn(lua_State *L, enum gru_error e)
{
  lgru_last_error = e;
  if (e && lgru_fatal_errors)
    return luaL_error(L, "libgru error %d: %s", e, gru_error_string(e));
  lua_pushnil(L);
  return 1;
}


static int lgru_gru_get_fatal_errors(lua_State *L)
{
  lua_pushboolean(L, lgru_fatal_errors);
  return 1;
}

static int lgru_gru_set_fatal_errors(lua_State *L)
{
  lgru_fatal_errors = lua_toboolean(L, 1);
  return 0;
}

static int lgru_gru_get_last_error(lua_State *L)
{
  lua_pushinteger(L, lgru_last_error);
  lua_pushstring(L, gru_error_string(lgru_last_error));
  return 2;
}

static int lgru_gru_blob_create(lua_State *L)
{
  if (!lgru_blob_create(L))
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  return 1;
}

static int lgru_gru_blob_load(lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  struct gru_blob *blob = lgru_blob_create(L);
  if (!blob)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_blob_load(blob, filename);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_gru_blob_string(lua_State *L)
{
  size_t size;
  const char *value = lua_tolstring(L, 1, &size);
  struct gru_blob *blob = lgru_blob_create(L);
  if (!blob)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_blob_writestring(blob, 0, size, value);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_gru_blob_blank(lua_State *L)
{
  size_t size = luaL_checkinteger(L, 1);
  struct gru_blob *blob = lgru_blob_create(L);
  if (!blob)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_blob_resize(blob, size);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_gru_n64rom_create(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_n64rom_create(L);
  if (!n64rom)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  return 1;
}

static int lgru_gru_n64rom_load(lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  struct gru_n64rom *n64rom = lgru_n64rom_create(L);
  if (!n64rom)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_n64rom_load_file(n64rom, filename);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_gru_z64fs_create(lua_State *L)
{
  if (!lgru_z64fs_create(L))
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  return 1;
}

static int lgru_gru_z64fs_load_file(lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  size_t ftab_start;
  size_t *ftab_start_ptr = NULL;
  if (lua_isinteger(L, 2)) {
    ftab_start = luaL_checkinteger(L, 2);
    ftab_start_ptr = &ftab_start;
  }
  struct gru_z64fs *z64fs = lgru_z64fs_create(L);
  if (!z64fs)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  struct gru_n64rom n64rom;
  gru_n64rom_init(&n64rom);
  enum gru_error e = gru_n64rom_load_file(&n64rom, filename);
  if (e) {
    gru_n64rom_destroy(&n64rom);
    return lgru_handle_error_noreturn(L, e);
  }
  e = gru_z64fs_load(z64fs, &n64rom.blob, ftab_start_ptr);
  if (e) {
    gru_n64rom_destroy(&n64rom);
    return lgru_handle_error_noreturn(L, e);
  }
  gru_n64rom_destroy(&n64rom);
  return 1;
}

static int lgru_gru_z64fs_load_blob(lua_State *L)
{
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  size_t ftab_start;
  size_t *ftab_start_ptr = NULL;
  if (lua_isinteger(L, 2)) {
    ftab_start = luaL_checkinteger(L, 2);
    ftab_start_ptr = &ftab_start;
  }
  struct gru_z64fs *z64fs = lgru_z64fs_create(L);
  if (!z64fs)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_z64fs_load(z64fs, blob, ftab_start_ptr);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_gru_gsc_create(lua_State *L)
{
  struct gru_gsc *gsc = lgru_gsc_create(L);
  if (!gsc)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  return 1;
}

static int lgru_gru_gsc_load(lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  struct gru_gsc *gsc = lgru_gsc_create(L);
  if (!gsc)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_gsc_load(gsc, filename);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_gru_ups_create(lua_State *L)
{
  struct gru_blob *src_blob = lgru_checkclass(L, 1, "gru_blob");
  struct gru_blob *dst_blob = lgru_checkclass(L, 2, "gru_blob");
  struct gru_ups *ups = lgru_ups_create(L);
  if (!ups)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_ups_make(ups, src_blob, dst_blob);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_gru_ups_load(lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  struct gru_ups *ups = lgru_ups_create(L);
  if (!ups)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_ups_load(ups, filename);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

void lgru_gru_register(lua_State *L)
{
  if (luaL_newmetatable(L, "gru_gru")) {
    /* constants */
    lua_pushinteger(L, GRU_SUCCESS);
    lua_setfield(L, -2, "SUCCESS");
    lua_pushinteger(L, GRU_ERROR_MEMORY);
    lua_setfield(L, -2, "ERROR_MEMORY");
    lua_pushinteger(L, GRU_ERROR_RANGE);
    lua_setfield(L, -2, "ERROR_RANGE");
    lua_pushinteger(L, GRU_ERROR_PARAM);
    lua_setfield(L, -2, "ERROR_PARAM");
    lua_pushinteger(L, GRU_ERROR_FILEIN);
    lua_setfield(L, -2, "ERROR_FILEIN");
    lua_pushinteger(L, GRU_ERROR_FILEOUT);
    lua_setfield(L, -2, "ERROR_FILEOUT");
    lua_pushinteger(L, GRU_ERROR_DATA);
    lua_setfield(L, -2, "ERROR_DATA");
    lua_pushinteger(L, GRU_ENDIAN_BIG_4);
    lua_setfield(L, -2, "ENDIAN_BIG_4");
    lua_pushinteger(L, GRU_ENDIAN_LITTLE_4);
    lua_setfield(L, -2, "ENDIAN_LITTLE_4");
    lua_pushinteger(L, GRU_ENDIAN_LITTLE_2);
    lua_setfield(L, -2, "ENDIAN_LITTLE_2");
    /* methods */
    lua_pushcfunction(L, lgru_gru_get_fatal_errors);
    lua_setfield(L, -2, "get_fatal_errors");
    lua_pushcfunction(L, lgru_gru_set_fatal_errors);
    lua_setfield(L, -2, "set_fatal_errors");
    lua_pushcfunction(L, lgru_gru_get_last_error);
    lua_setfield(L, -2, "get_last_error");
    lua_pushcfunction(L, lgru_gru_blob_create);
    lua_setfield(L, -2, "blob_create");
    lua_pushcfunction(L, lgru_gru_blob_load);
    lua_setfield(L, -2, "blob_load");
    lua_pushcfunction(L, lgru_gru_blob_string);
    lua_setfield(L, -2, "blob_string");
    lua_pushcfunction(L, lgru_gru_blob_blank);
    lua_setfield(L, -2, "blob_blank");
    lua_pushcfunction(L, lgru_gru_n64rom_create);
    lua_setfield(L, -2, "n64rom_create");
    lua_pushcfunction(L, lgru_gru_n64rom_load);
    lua_setfield(L, -2, "n64rom_load");
    lua_pushcfunction(L, lgru_gru_z64fs_create);
    lua_setfield(L, -2, "z64fs_create");
    lua_pushcfunction(L, lgru_gru_z64fs_load_file);
    lua_setfield(L, -2, "z64fs_load_file");
    lua_pushcfunction(L, lgru_gru_z64fs_load_blob);
    lua_setfield(L, -2, "z64fs_load_blob");
    lua_pushcfunction(L, lgru_gru_gsc_create);
    lua_setfield(L, -2, "gsc_create");
    lua_pushcfunction(L, lgru_gru_gsc_load);
    lua_setfield(L, -2, "gsc_load");
    lua_pushcfunction(L, lgru_gru_ups_create);
    lua_setfield(L, -2, "ups_create");
    lua_pushcfunction(L, lgru_gru_ups_load);
    lua_setfield(L, -2, "ups_load");
    lua_pushcfunction(L, lgru_os_rm);
    lua_setfield(L, -2, "os_rm");
    /* meta */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
  }
}

void lgru_gru_create(lua_State *L)
{
  lua_newtable(L);
  lgru_gru_register(L);
  lua_setmetatable(L, -2);
}
