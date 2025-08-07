#if defined(HAVE_LUA5_4_LUA_H)
#include <lua5.4/lualib.h>
#elif defined(HAVE_LUA54_LUA_H)
#include <lua54/lualib.h>
#elif defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lualib.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lualib.h>
#elif defined(HAVE_LUA_H)
#include <lualib.h>
#endif
