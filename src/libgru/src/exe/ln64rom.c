#include <config.h>
#ifdef HAVE_LUA5_3_LUA_H
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#else
#include <lua.h>
#include <lauxlib.h>
#endif
#include "lgru.h"
#include "lblob.h"
#include "../lib/libgru.h"

static int lgru_n64rom_load_file(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  const char *filename = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_n64rom_load_file(n64rom, filename));
}

static int lgru_n64rom_load_blob(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  struct gru_blob *blob = lgru_checkclass(L, 1, "gru_blob");
  return lgru_handle_error(L, gru_n64rom_load_blob(n64rom, blob));
}

static int lgru_n64rom_save_file(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  const char *filename = luaL_checkstring(L, 2);
  enum gru_endian endian = luaL_optinteger(L, 3, GRU_ENDIAN_BIG_4);
  return lgru_handle_error(L, gru_n64rom_save_file(n64rom, filename, endian));
}

static int lgru_n64rom_save_blob(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  enum gru_endian endian = luaL_optinteger(L, 2, GRU_ENDIAN_BIG_4);
  struct gru_blob *blob = lgru_blob_create(L);
  enum gru_error e = gru_n64rom_save_blob(n64rom, blob, endian);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_n64rom_clock_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t clock;
  enum gru_error e = gru_n64rom_clock_get(n64rom, &clock);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, clock);
  return 1;
}

static int lgru_n64rom_clock_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t clock = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_n64rom_clock_set(n64rom, clock));
}

static int lgru_n64rom_pc_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t pc;
  enum gru_error e = gru_n64rom_pc_get(n64rom, &pc);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, pc);
  return 1;
}

static int lgru_n64rom_pc_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t pc = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_n64rom_pc_set(n64rom, pc));
}

static int lgru_n64rom_release_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t release;
  enum gru_error e = gru_n64rom_release_get(n64rom, &release);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, release);
  return 1;
}

static int lgru_n64rom_release_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t release = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_n64rom_release_set(n64rom, release));
}

static int lgru_n64rom_crc_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t crc1;
  uint32_t crc2;
  enum gru_error e = gru_n64rom_crc_get(n64rom, &crc1, &crc2);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, crc1);
  lua_pushinteger(L, crc2);
  return 2;
}

static int lgru_n64rom_crc_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t crc1 = luaL_checkinteger(L, 2);
  uint32_t crc2 = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_n64rom_crc_set(n64rom, crc1, crc2));
}

static int lgru_n64rom_name_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  char name[20];
  enum gru_error e = gru_n64rom_name_get(n64rom, name);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushlstring(L, name, 20);
  return 1;
}

static int lgru_n64rom_name_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  const char *name = luaL_checkstring(L, 2);
  return lgru_handle_error(L, gru_n64rom_name_set(n64rom, name));
}

static int lgru_n64rom_format_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  char format;
  enum gru_error e = gru_n64rom_format_get(n64rom, &format);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushlstring(L, &format, 1);
  return 1;
}

static int lgru_n64rom_format_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  size_t format_size;
  const char *format = luaL_checklstring(L, 2, &format_size);
  if (format_size != 1)
    return lgru_handle_error(L, GRU_ERROR_PARAM);
  return lgru_handle_error(L, gru_n64rom_format_set(n64rom, *format));
}

static int lgru_n64rom_id_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  char id[2];
  enum gru_error e = gru_n64rom_id_get(n64rom, id);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushlstring(L, id, 2);
  return 1;
}

static int lgru_n64rom_id_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  size_t id_size;
  const char *id = luaL_checklstring(L, 2, &id_size);
  if (id_size != 2)
    return lgru_handle_error(L, GRU_ERROR_PARAM);
  return lgru_handle_error(L, gru_n64rom_id_set(n64rom, id));
}

static int lgru_n64rom_region_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  char region;
  enum gru_error e = gru_n64rom_region_get(n64rom, &region);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushlstring(L, &region, 1);
  return 1;
}

static int lgru_n64rom_region_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  size_t region_size;
  const char *region = luaL_checklstring(L, 2, &region_size);
  if (region_size != 1)
    return lgru_handle_error(L, GRU_ERROR_PARAM);
  return lgru_handle_error(L, gru_n64rom_region_set(n64rom, *region));
}

static int lgru_n64rom_version_get(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint8_t version;
  enum gru_error e = gru_n64rom_version_get(n64rom, &version);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, version);
  return 1;
}

static int lgru_n64rom_version_set(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint8_t version = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_n64rom_version_set(n64rom, version));
}

static int lgru_n64rom_cic(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  int cic = gru_n64rom_cic(n64rom);
  lua_pushinteger(L, cic);
  return 1;
}

static int lgru_n64rom_crc_compute(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  uint32_t crc1;
  uint32_t crc2;
  gru_n64rom_crc_compute(n64rom, &crc1, &crc2);
  lua_pushinteger(L, crc1);
  lua_pushinteger(L, crc2);
  return 2;
}

static int lgru_n64rom_crc_check(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  lua_pushboolean(L, gru_n64rom_crc_check(n64rom));
  return 1;
}

static int lgru_n64rom_crc_update(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  return lgru_handle_error(L, gru_n64rom_crc_update(n64rom));
}

static int lgru_n64rom_gc(lua_State *L)
{
  struct gru_n64rom *n64rom = lgru_checkclass(L, 1, "gru_n64rom");
  gru_n64rom_destroy(n64rom);
  return 0;
}

void lgru_n64rom_register(lua_State *L)
{
  if (luaL_newmetatable(L, "gru_n64rom")) {
    /* methods */
    lua_pushcfunction(L, lgru_n64rom_load_file);
    lua_setfield(L, -2, "load_file");
    lua_pushcfunction(L, lgru_n64rom_load_blob);
    lua_setfield(L, -2, "load_blob");
    lua_pushcfunction(L, lgru_n64rom_save_file);
    lua_setfield(L, -2, "save_file");
    lua_pushcfunction(L, lgru_n64rom_save_blob);
    lua_setfield(L, -2, "save_blob");
    lua_pushcfunction(L, lgru_n64rom_clock_set);
    lua_setfield(L, -2, "clock_set");
    lua_pushcfunction(L, lgru_n64rom_clock_get);
    lua_setfield(L, -2, "clock_get");
    lua_pushcfunction(L, lgru_n64rom_pc_set);
    lua_setfield(L, -2, "pc_set");
    lua_pushcfunction(L, lgru_n64rom_pc_get);
    lua_setfield(L, -2, "pc_get");
    lua_pushcfunction(L, lgru_n64rom_release_set);
    lua_setfield(L, -2, "release_set");
    lua_pushcfunction(L, lgru_n64rom_release_get);
    lua_setfield(L, -2, "release_get");
    lua_pushcfunction(L, lgru_n64rom_crc_set);
    lua_setfield(L, -2, "crc_set");
    lua_pushcfunction(L, lgru_n64rom_crc_get);
    lua_setfield(L, -2, "crc_get");
    lua_pushcfunction(L, lgru_n64rom_name_set);
    lua_setfield(L, -2, "name_set");
    lua_pushcfunction(L, lgru_n64rom_name_get);
    lua_setfield(L, -2, "name_get");
    lua_pushcfunction(L, lgru_n64rom_format_set);
    lua_setfield(L, -2, "format_set");
    lua_pushcfunction(L, lgru_n64rom_format_get);
    lua_setfield(L, -2, "format_get");
    lua_pushcfunction(L, lgru_n64rom_id_set);
    lua_setfield(L, -2, "id_set");
    lua_pushcfunction(L, lgru_n64rom_id_get);
    lua_setfield(L, -2, "id_get");
    lua_pushcfunction(L, lgru_n64rom_region_set);
    lua_setfield(L, -2, "region_set");
    lua_pushcfunction(L, lgru_n64rom_region_get);
    lua_setfield(L, -2, "region_get");
    lua_pushcfunction(L, lgru_n64rom_version_set);
    lua_setfield(L, -2, "version_set");
    lua_pushcfunction(L, lgru_n64rom_version_get);
    lua_setfield(L, -2, "version_get");
    lua_pushcfunction(L, lgru_n64rom_cic);
    lua_setfield(L, -2, "cic");
    lua_pushcfunction(L, lgru_n64rom_crc_compute);
    lua_setfield(L, -2, "crc_compute");
    lua_pushcfunction(L, lgru_n64rom_crc_check);
    lua_setfield(L, -2, "crc_check");
    lua_pushcfunction(L, lgru_n64rom_crc_update);
    lua_setfield(L, -2, "crc_update");
    /* meta */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lgru_n64rom_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
    /* class inheritance */
    lua_newtable(L);
    lgru_blob_register(L);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
  }
}

struct gru_n64rom *lgru_n64rom_create(lua_State *L)
{
  struct gru_n64rom *n64rom = lua_newuserdata(L, sizeof(struct gru_n64rom));
  gru_n64rom_init(n64rom);
  lgru_n64rom_register(L);
  lua_setmetatable(L, -2);
  return n64rom;
}
