#include <lua.h>
#include <lauxlib.h>
#include "lgru.h"
#include "lblob.h"
#include "ln64rom.h"
#include "../lib/libgru.h"

static int lgru_z64fs_load(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  struct gru_blob *blob = lgru_checkclass(L, 2, "gru_blob");
  size_t ftab_start;
  size_t *ftab_start_ptr = NULL;
  if (lua_isinteger(L, 3)) {
    ftab_start = luaL_checkinteger(L, 3);
    ftab_start_ptr = &ftab_start;
  }
  return lgru_handle_error(L, gru_z64fs_load(z64fs, blob, ftab_start_ptr));
}

static int lgru_z64fs_assemble_blob(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  struct gru_blob *blob = lgru_blob_create(L);
  if (!blob)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_z64fs_assemble_blob(z64fs, blob);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_z64fs_assemble_rom(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  struct gru_n64rom *n64rom = lgru_n64rom_create(L);
  if (!n64rom)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_z64fs_assemble_rom(z64fs, n64rom);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_z64fs_length(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_length(z64fs));
  return 1;
}

static int lgru_z64fs_ftab(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_ftab(z64fs));
  return 1;
}

static int lgru_z64fs_ftab_vvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushboolean(L, gru_z64fs_ftab_vvolatile(z64fs));
  return 1;
}

static int lgru_z64fs_set_ftab_vvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  gru_bool_t ftab_vvolatile = lua_toboolean(L, 2);
  gru_z64fs_set_ftab_vvolatile(z64fs, ftab_vvolatile);
  return 0;
}

static int lgru_z64fs_ftab_pvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushboolean(L, gru_z64fs_ftab_pvolatile(z64fs));
  return 1;
}

static int lgru_z64fs_set_ftab_pvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  gru_bool_t ftab_pvolatile = lua_toboolean(L, 2);
  gru_z64fs_set_ftab_pvolatile(z64fs, ftab_pvolatile);
  return 0;
}

static int lgru_z64fs_vrom_first(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_vrom_first(z64fs));
  return 1;
}

static int lgru_z64fs_vrom_last(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_vrom_last(z64fs));
  return 1;
}

static int lgru_z64fs_prom_first(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_prom_first(z64fs));
  return 1;
}

static int lgru_z64fs_prom_last(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_prom_last(z64fs));
  return 1;
}

static int lgru_z64fs_vrom_size(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_vrom_size(z64fs));
  return 1;
}

static int lgru_z64fs_vrom_tail(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_vrom_tail(z64fs));
  return 1;
}

static int lgru_z64fs_prom_size(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_prom_size(z64fs));
  return 1;
}

static int lgru_z64fs_set_prom_size(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t prom_size = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_z64fs_set_prom_size(z64fs, prom_size));
}

static int lgru_z64fs_prom_tail(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushinteger(L, gru_z64fs_prom_tail(z64fs));
  return 1;
}

static int lgru_z64fs_vvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushboolean(L, gru_z64fs_vvolatile(z64fs));
  return 1;
}

static int lgru_z64fs_set_vvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  gru_bool_t vrom_volatile = lua_toboolean(L, 2);
  gru_z64fs_set_vvolatile(z64fs, vrom_volatile);
  return 0;
}

static int lgru_z64fs_pvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  lua_pushboolean(L, gru_z64fs_pvolatile(z64fs));
  return 1;
}

static int lgru_z64fs_set_pvolatile(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  gru_bool_t prom_volatile = lua_toboolean(L, 2);
  gru_z64fs_set_pvolatile(z64fs, prom_volatile);
  return 0;
}

static int lgru_z64fs_vfind(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t vrom_start = luaL_checkinteger(L, 2);
  size_t vrom_end = luaL_checkinteger(L, 3);
  size_t index;
  if (gru_z64fs_vfind(z64fs, vrom_start, vrom_end, &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_pfind(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t prom_start = luaL_checkinteger(L, 2);
  size_t prom_end = luaL_checkinteger(L, 3);
  size_t index;
  if (gru_z64fs_pfind(z64fs, prom_start, prom_end, &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_vat(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t vrom_address = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_vat(z64fs, vrom_address,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_pat(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t prom_address = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_vat(z64fs, prom_address,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_vindex(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t vrom_order = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_vindex(z64fs, vrom_order,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_pindex(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t prom_order = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_pindex(z64fs, prom_order,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_vprev(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t vrom_order = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_vprev(z64fs, vrom_order,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_vnext(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t vrom_order = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_vnext(z64fs, vrom_order,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_pprev(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t prom_order = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_pprev(z64fs, prom_order,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_pnext(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t prom_order = luaL_checkinteger(L, 2);
  size_t index;
  if (gru_z64fs_pnext(z64fs, prom_order,  &index))
    lua_pushnil(L);
  else
    lua_pushinteger(L, index);
  return 1;
}

static int lgru_z64fs_insert(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  struct gru_blob *blob;
  if (lua_isnoneornil(L, 2))
    blob = NULL;
  else
    blob = lgru_checkclass(L, 2, "gru_blob");
  gru_bool_t compress = lua_toboolean(L, 3);
  enum gru_error e = gru_z64fs_insert(z64fs, blob, compress);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, gru_z64fs_length(z64fs) - 1);
  return 1;
}

static int lgru_z64fs_replace(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  struct gru_blob *blob;
  if (lua_isnoneornil(L, 3))
    blob = NULL;
  else
    blob = lgru_checkclass(L, 3, "gru_blob");
  gru_bool_t compress = lua_toboolean(L, 4);
  return lgru_handle_error(L, gru_z64fs_replace(z64fs, index, blob, compress));
}

static int lgru_z64fs_remove(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_z64fs_remove(z64fs, index));
}

static int lgru_z64fs_nullify(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  return lgru_handle_error(L, gru_z64fs_nullify(z64fs, index));
}

static int lgru_z64fs_get(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  gru_bool_t decompress = lua_isnoneornil(L, 3) || lua_toboolean(L, 3);
  gru_bool_t null;
  enum gru_error e = gru_z64fs_null(z64fs, index, &null);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  if (null)
    return lgru_handle_error_noreturn(L, GRU_SUCCESS);
  struct gru_blob *blob = lgru_blob_create(L);
  e = gru_z64fs_get(z64fs, index, decompress, blob);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_z64fs_vstart(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_start;
  enum gru_error e = gru_z64fs_vstart(z64fs, index, &vrom_start);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, vrom_start);
  return 1;
}

static int lgru_z64fs_vend(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_end;
  enum gru_error e = gru_z64fs_vend(z64fs, index, &vrom_end);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, vrom_end);
  return 1;
}

static int lgru_z64fs_vsize(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_size;
  enum gru_error e = gru_z64fs_vsize(z64fs, index, &vrom_size);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, vrom_size);
  return 1;
}

static int lgru_z64fs_set_vsize(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_size = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_vsize(z64fs, index, vrom_size));
}

static int lgru_z64fs_vpadding(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_padding_size;
  enum gru_error e = gru_z64fs_vpadding(z64fs, index, &vrom_padding_size);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, vrom_padding_size);
  return 1;
}

static int lgru_z64fs_set_vpadding(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_padding_size = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_vpadding(z64fs, index,
                                                     vrom_padding_size));
}

static int lgru_z64fs_vspace(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_space;
  enum gru_error e = gru_z64fs_vspace(z64fs, index, &vrom_space);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, vrom_space);
  return 1;
}

static int lgru_z64fs_set_vspace(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_space = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_vspace(z64fs, index, vrom_space));
}

static int lgru_z64fs_vorder(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_order;
  enum gru_error e = gru_z64fs_vorder(z64fs, index, &vrom_order);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, vrom_order);
  return 1;
}

static int lgru_z64fs_set_vorder(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t vrom_order = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_vorder(z64fs, index, vrom_order));
}

static int lgru_z64fs_pstart(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_start;
  enum gru_error e = gru_z64fs_pstart(z64fs, index, &prom_start);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, prom_start);
  return 1;
}

static int lgru_z64fs_pend(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_end;
  enum gru_error e = gru_z64fs_pend(z64fs, index, &prom_end);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, prom_end);
  return 1;
}

static int lgru_z64fs_ptail(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_tail;
  enum gru_error e = gru_z64fs_ptail(z64fs, index, &prom_tail);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, prom_tail);
  return 1;
}

static int lgru_z64fs_psize(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_size;
  enum gru_error e = gru_z64fs_psize(z64fs, index, &prom_size);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, prom_size);
  return 1;
}

static int lgru_z64fs_ppadding(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_padding_size;
  enum gru_error e = gru_z64fs_ppadding(z64fs, index, &prom_padding_size);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, prom_padding_size);
  return 1;
}

static int lgru_z64fs_get_ppadding(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  struct gru_blob *blob = lgru_blob_create(L);
  if (!blob)
    return lgru_handle_error_noreturn(L, GRU_ERROR_MEMORY);
  enum gru_error e = gru_z64fs_get_ppadding(z64fs, index, blob);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  return 1;
}

static int lgru_z64fs_set_ppadding(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  struct gru_blob *blob = lgru_checkclass(L, 3, "gru_blob");
  return lgru_handle_error(L, gru_z64fs_set_ppadding(z64fs, index, blob));
}

static int lgru_z64fs_pspace(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_space;
  enum gru_error e = gru_z64fs_pspace(z64fs, index, &prom_space);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, prom_space);
  return 1;
}

static int lgru_z64fs_set_pspace(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_space = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_pspace(z64fs, index, prom_space));
}

static int lgru_z64fs_porder(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_order;
  enum gru_error e = gru_z64fs_porder(z64fs, index, &prom_order);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushinteger(L, prom_order);
  return 1;
}

static int lgru_z64fs_set_porder(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t prom_order = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_porder(z64fs, index, prom_order));
}

static int lgru_z64fs_set_index(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  size_t new_index = luaL_checkinteger(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_index(z64fs, index, new_index));
}

static int lgru_z64fs_compressed(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  gru_bool_t compressed;
  enum gru_error e = gru_z64fs_compressed(z64fs, index, &compressed);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushboolean(L, compressed);
  return 1;
}

static int lgru_z64fs_set_compressed(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  gru_bool_t compressed = lua_toboolean(L, 3);
  return lgru_handle_error(L, gru_z64fs_set_compressed(z64fs, index,
                                                       compressed));
}

static int lgru_z64fs_null(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  size_t index = luaL_checkinteger(L, 2);
  gru_bool_t null;
  enum gru_error e = gru_z64fs_null(z64fs, index, &null);
  if (e)
    return lgru_handle_error_noreturn(L, e);
  lua_pushboolean(L, null);
  return 1;
}

static int lgru_z64fs_gc(lua_State *L)
{
  struct gru_z64fs *z64fs = lgru_checkclass(L, 1, "gru_z64fs");
  gru_z64fs_destroy(z64fs);
  return 0;
}

void lgru_z64fs_register(lua_State *L)
{
  if (luaL_newmetatable(L, "gru_z64fs")) {
    /* methods */
    lua_pushcfunction(L, lgru_z64fs_load);
    lua_setfield(L, -2, "load");
    lua_pushcfunction(L, lgru_z64fs_assemble_blob);
    lua_setfield(L, -2, "assemble_blob");
    lua_pushcfunction(L, lgru_z64fs_assemble_rom);
    lua_setfield(L, -2, "assemble_rom");
    lua_pushcfunction(L, lgru_z64fs_length);
    lua_setfield(L, -2, "length");
    lua_pushcfunction(L, lgru_z64fs_ftab);
    lua_setfield(L, -2, "ftab");
    lua_pushcfunction(L, lgru_z64fs_ftab_vvolatile);
    lua_setfield(L, -2, "ftab_vvolatile");
    lua_pushcfunction(L, lgru_z64fs_set_ftab_vvolatile);
    lua_setfield(L, -2, "set_ftab_vvolatile");
    lua_pushcfunction(L, lgru_z64fs_ftab_pvolatile);
    lua_setfield(L, -2, "ftab_pvolatile");
    lua_pushcfunction(L, lgru_z64fs_set_ftab_pvolatile);
    lua_setfield(L, -2, "set_ftab_pvolatile");
    lua_pushcfunction(L, lgru_z64fs_vrom_first);
    lua_setfield(L, -2, "vrom_first");
    lua_pushcfunction(L, lgru_z64fs_vrom_last);
    lua_setfield(L, -2, "vrom_last");
    lua_pushcfunction(L, lgru_z64fs_prom_first);
    lua_setfield(L, -2, "prom_first");
    lua_pushcfunction(L, lgru_z64fs_prom_last);
    lua_setfield(L, -2, "prom_last");
    lua_pushcfunction(L, lgru_z64fs_vrom_size);
    lua_setfield(L, -2, "vrom_size");
    lua_pushcfunction(L, lgru_z64fs_vrom_tail);
    lua_setfield(L, -2, "vrom_tail");
    lua_pushcfunction(L, lgru_z64fs_prom_size);
    lua_setfield(L, -2, "prom_size");
    lua_pushcfunction(L, lgru_z64fs_set_prom_size);
    lua_setfield(L, -2, "set_prom_size");
    lua_pushcfunction(L, lgru_z64fs_prom_tail);
    lua_setfield(L, -2, "prom_tail");
    lua_pushcfunction(L, lgru_z64fs_vvolatile);
    lua_setfield(L, -2, "vvolatile");
    lua_pushcfunction(L, lgru_z64fs_set_vvolatile);
    lua_setfield(L, -2, "set_vvolatile");
    lua_pushcfunction(L, lgru_z64fs_pvolatile);
    lua_setfield(L, -2, "pvolatile");
    lua_pushcfunction(L, lgru_z64fs_set_pvolatile);
    lua_setfield(L, -2, "set_pvolatile");
    lua_pushcfunction(L, lgru_z64fs_vfind);
    lua_setfield(L, -2, "vfind");
    lua_pushcfunction(L, lgru_z64fs_pfind);
    lua_setfield(L, -2, "pfind");
    lua_pushcfunction(L, lgru_z64fs_vat);
    lua_setfield(L, -2, "vat");
    lua_pushcfunction(L, lgru_z64fs_pat);
    lua_setfield(L, -2, "pat");
    lua_pushcfunction(L, lgru_z64fs_vindex);
    lua_setfield(L, -2, "vindex");
    lua_pushcfunction(L, lgru_z64fs_pindex);
    lua_setfield(L, -2, "pindex");
    lua_pushcfunction(L, lgru_z64fs_vprev);
    lua_setfield(L, -2, "vprev");
    lua_pushcfunction(L, lgru_z64fs_vnext);
    lua_setfield(L, -2, "vnext");
    lua_pushcfunction(L, lgru_z64fs_pprev);
    lua_setfield(L, -2, "pprev");
    lua_pushcfunction(L, lgru_z64fs_pnext);
    lua_setfield(L, -2, "pnext");
    lua_pushcfunction(L, lgru_z64fs_insert);
    lua_setfield(L, -2, "insert");
    lua_pushcfunction(L, lgru_z64fs_replace);
    lua_setfield(L, -2, "replace");
    lua_pushcfunction(L, lgru_z64fs_remove);
    lua_setfield(L, -2, "remove");
    lua_pushcfunction(L, lgru_z64fs_nullify);
    lua_setfield(L, -2, "nullify");
    lua_pushcfunction(L, lgru_z64fs_get);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, lgru_z64fs_vstart);
    lua_setfield(L, -2, "vstart");
    lua_pushcfunction(L, lgru_z64fs_vend);
    lua_setfield(L, -2, "vend");
    lua_pushcfunction(L, lgru_z64fs_vsize);
    lua_setfield(L, -2, "vsize");
    lua_pushcfunction(L, lgru_z64fs_set_vsize);
    lua_setfield(L, -2, "set_vsize");
    lua_pushcfunction(L, lgru_z64fs_vpadding);
    lua_setfield(L, -2, "vpadding");
    lua_pushcfunction(L, lgru_z64fs_set_vpadding);
    lua_setfield(L, -2, "set_vpadding");
    lua_pushcfunction(L, lgru_z64fs_vspace);
    lua_setfield(L, -2, "vspace");
    lua_pushcfunction(L, lgru_z64fs_set_vspace);
    lua_setfield(L, -2, "set_vspace");
    lua_pushcfunction(L, lgru_z64fs_vorder);
    lua_setfield(L, -2, "vorder");
    lua_pushcfunction(L, lgru_z64fs_set_vorder);
    lua_setfield(L, -2, "set_vorder");
    lua_pushcfunction(L, lgru_z64fs_pstart);
    lua_setfield(L, -2, "pstart");
    lua_pushcfunction(L, lgru_z64fs_pend);
    lua_setfield(L, -2, "pend");
    lua_pushcfunction(L, lgru_z64fs_ptail);
    lua_setfield(L, -2, "ptail");
    lua_pushcfunction(L, lgru_z64fs_psize);
    lua_setfield(L, -2, "psize");
    lua_pushcfunction(L, lgru_z64fs_ppadding);
    lua_setfield(L, -2, "ppadding");
    lua_pushcfunction(L, lgru_z64fs_get_ppadding);
    lua_setfield(L, -2, "get_ppadding");
    lua_pushcfunction(L, lgru_z64fs_set_ppadding);
    lua_setfield(L, -2, "set_ppadding");
    lua_pushcfunction(L, lgru_z64fs_pspace);
    lua_setfield(L, -2, "pspace");
    lua_pushcfunction(L, lgru_z64fs_set_pspace);
    lua_setfield(L, -2, "set_pspace");
    lua_pushcfunction(L, lgru_z64fs_porder);
    lua_setfield(L, -2, "porder");
    lua_pushcfunction(L, lgru_z64fs_set_porder);
    lua_setfield(L, -2, "set_porder");
    lua_pushcfunction(L, lgru_z64fs_set_index);
    lua_setfield(L, -2, "set_index");
    lua_pushcfunction(L, lgru_z64fs_compressed);
    lua_setfield(L, -2, "compressed");
    lua_pushcfunction(L, lgru_z64fs_set_compressed);
    lua_setfield(L, -2, "set_compressed");
    lua_pushcfunction(L, lgru_z64fs_null);
    lua_setfield(L, -2, "null");
    /* meta */
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lgru_z64fs_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
  }
}

struct gru_z64fs *lgru_z64fs_create(lua_State *L)
{
  struct gru_z64fs *z64fs = lua_newuserdata(L, sizeof(struct gru_z64fs));
  if (gru_z64fs_init(z64fs)) {
    lua_pop(L, 1);
    return NULL;
  }
  lgru_z64fs_register(L);
  lua_setmetatable(L, -2);
  return z64fs;
}
