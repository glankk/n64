#include <config.h>
#include <stdio.h>
#include <string.h>
#if defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#include <lua5.3/lualib.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lua.h>
#include <lua53/lauxlib.h>
#include <lua53/lualib.h>
#elif defined(HAVE_LUA_H)
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#endif
#include "lgru.h"

struct lgru_builtin
{
  const char *name;
  const char *usage;
  const char *script;
}
static builtins[] =
{
  {
    "gru_crc32",
    "gru gru_crc32 <file>",
    "local arg = {...}                                \n"
    "local file = gru.blob_load(arg[1])               \n"
    "print(string.format(\"0x%08X\", file:crc32()))   \n",
  },
  {
    "gru_swap",
    "gru gru_swap <file> <word-size>",
    "local arg = {...}                                \n"
    "local file = gru.blob_load(arg[1])               \n"
    "file:swap(arg[2])                                \n"
    "file:save(arg[1])                                \n",
  },
  {
    "gru_ups_create",
    "gru gru_ups_create <original-file> <modified-file> <output-file>",
    "local arg = {...}                                \n"
    "local src_file = gru.blob_load(arg[1])           \n"
    "local dst_file = gru.blob_load(arg[2])           \n"
    "local ups = gru.ups_create(src_file, dst_file)   \n"
    "ups:save(arg[3])                                 \n",
  },
  {
    "gru_ups_apply",
    "gru gru_ups_apply <patch-file> <input-file> <output-file>",
    "local arg = {...}                                \n"
    "local ups = gru.ups_load(arg[1])                 \n"
    "local file = gru.blob_load(arg[2])               \n"
    "ups:apply(file)                                  \n"
    "file:save(arg[3])                                \n",
  },
  {
    "gru_ups_undo",
    "gru gru_ups_undo <patch-file> <input-file> <output-file>",
    "local arg = {...}                                \n"
    "local ups = gru.ups_load(arg[1])                 \n"
    "local file = gru.blob_load(arg[2])               \n"
    "ups:undo(file)                                   \n"
    "file:save(arg[3])                                \n",
  },
};

int main(int argc, char *argv[])
{
  if (argc > 1) {
    /* lua initialization */
    lua_State *L = luaL_newstate();
    if (!L) {
      fprintf(stderr, "failed to create lua state\n");
      return LUA_ERRMEM;
    }
    luaL_requiref(L, "_G", luaopen_base, 1);
    luaL_requiref(L, "package", luaopen_package, 1);
    luaL_requiref(L, "coroutine", luaopen_coroutine, 1);
    luaL_requiref(L, "string", luaopen_string, 1);
    luaL_requiref(L, "utf8", luaopen_utf8, 1);
    luaL_requiref(L, "table", luaopen_table, 1);
    luaL_requiref(L, "math", luaopen_math, 1);
    luaL_requiref(L, "io", luaopen_io, 1);
    luaL_requiref(L, "os", luaopen_os, 1);
    lua_settop(L, 0);
    /* main interface object initialization */
    lgru_gru_create(L);
    lua_setglobal(L, "gru");
    /* script execution */
    int is_builtin = 0;
    int e;
    for (size_t i = 0; i < sizeof(builtins) / sizeof(*builtins); ++i) {
      struct lgru_builtin *builtin = &builtins[i];
      if (strcmp(argv[1], builtin->name) == 0) {
        is_builtin = 1;
        e = luaL_loadbuffer(L, builtin->script, strlen(builtin->script),
                            builtin->name);
        break;
      }
    }
    if (!is_builtin)
      e = luaL_loadfile(L, argv[1]);
    if (e) {
      fprintf(stderr, "failed to load script `%s`\n", argv[1]);
      return e;
    }
    for (int i = 2; i < argc; ++i)
      lua_pushstring(L, argv[i]);
    e = lua_pcall(L, argc - 2, LUA_MULTRET, 0);
    if (e)
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
    else if (lua_gettop(L) > 0)
      e = lua_tointeger(L, 1);
    lua_close(L);
    return e;
  }
  else {
    puts("libgru-0.1\n"
         "build date: " __TIME__ ", " __DATE__ "\n"
         "written by: glank\n"
         "invocation: `gru <script> [args...]`\n"
         "the following builtin scripts are available:");
    for (size_t i = 0; i < sizeof(builtins) / sizeof(*builtins); ++i) {
      struct lgru_builtin *builtin = &builtins[i];
      printf("  `%s`\n", builtin->usage);
    }
    return 0;
  }
}
