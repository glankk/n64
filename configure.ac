#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT
AC_CANONICAL_HOST
AC_CONFIG_COMMANDS_PRE([case $host_os in mingw32|msys) LDFLAGS="$LDFLAGS -static";; esac])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P

# Checks for libraries.
AC_SEARCH_LIBS([pow], [m])
AC_SEARCH_LIBS([luaopen_utf8], [lua5.3 lua])

# Checks for header files.
AC_CHECK_HEADERS([lua5.3/lua.h lua.h])

AC_CONFIG_HEADERS([src/libgru/config.h])
AC_CONFIG_FILES([makefile src/gfxdis/makefile src/grc/makefile src/libgru/makefile src/libgs/makefile src/luapatch/makefile])
AC_OUTPUT
