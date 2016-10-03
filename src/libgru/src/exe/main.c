#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
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
    R"(
       local arg = {...}
       local file = gru.blob_load(arg[1])
       print(string.format("0x%08X", file:crc32()))
       )",
  },
  {
    "gru_swap",
    "gru gru_swap <file> <word-size>",
    R"(
       local arg = {...}
       local file = gru.blob_load(arg[1])
       file:swap(arg[2])
       file:save(arg[1])
       )",
  },
  {
    "gru_ups_create",
    "gru gru_ups_create <original-file> <modified-file> <output-file>",
    R"(
       local arg = {...}
       local src_file = gru.blob_load(arg[1])
       local dst_file = gru.blob_load(arg[2])
       local ups = gru.ups_create(src_file, dst_file)
       ups:save(arg[3])
       )",
  },
  {
    "gru_ups_apply",
    "gru gru_ups_apply <patch-file> <input-file> <output-file>",
    R"(
       local arg = {...}
       local ups = gru.ups_load(arg[1])
       local file = gru.blob_load(arg[2])
       ups:apply(file)
       file:save(arg[3])
       )",
  },
  {
    "gru_ups_undo",
    "gru gru_ups_undo <patch-file> <input-file> <output-file>",
    R"(
       local arg = {...}
       local ups = gru.ups_load(arg[1])
       local file = gru.blob_load(arg[2])
       ups:undo(file)
       file:save(arg[3])
       )",
  },
};

int main(int argc, char *argv[])
{
  if (argc > 1) {
    /* lua initialization */
    lua_State *L = luaL_newstate();
    if (!L) {
      puts("failed to create lua state");
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
      printf("failed to load script `%s`\n", argv[1]);
      return e;
    }
    for (int i = 2; i < argc; ++i)
      lua_pushstring(L, argv[i]);
    e = lua_pcall(L, argc - 2, LUA_MULTRET, 0);
    if (e)
      puts(lua_tostring(L, -1));
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
