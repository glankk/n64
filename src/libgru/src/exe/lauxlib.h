#if defined(HAVE_LUA5_4_LUA_H)
#include <lua5.4/lauxlib.h>
#elif defined(HAVE_LUA54_LUA_H)
#include <lua54/lauxlib.h>
#elif defined(HAVE_LUA5_3_LUA_H)
#include <lua5.3/lauxlib.h>
#elif defined(HAVE_LUA53_LUA_H)
#include <lua53/lauxlib.h>
#elif defined(HAVE_LUA_H)
#include <lauxlib.h>
#endif
