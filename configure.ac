#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT
AC_PREFIX_DEFAULT([/opt/n64])

# Checks for options.
AC_ARG_ENABLE([static-executables],
              [AS_HELP_STRING([--enable-static-executables],
                              [enable static linking of executables])],
              [LDFLAGS="-static $LDFLAGS"])
AC_SUBST([CFLAGS_FOR_TARGET])
AC_SUBST([CXXFLAGS_FOR_TARGET])
CFLAGS_FOR_TARGET="-g -ffunction-sections -fdata-sections $CFLAGS_FOR_TARGET"
CXXFLAGS_FOR_TARGET="-g -ffunction-sections -fdata-sections $CXXFLAGS_FOR_TARGET"
AC_ARG_ENABLE([vc], [AS_HELP_STRING([--enable-vc], [compile libraries with wii vc compatibility])],
[
  CFLAGS_FOR_TARGET="-O1 -fno-reorder-blocks $CFLAGS_FOR_TARGET"
  CXXFLAGS_FOR_TARGET="-O1 -fno-reorder-blocks $CXXFLAGS_FOR_TARGET"
],
[
  CFLAGS_FOR_TARGET="-Os $CFLAGS_FOR_TARGET"
  CXXFLAGS_FOR_TARGET="-Os $CXXFLAGS_FOR_TARGET"
])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_SEARCH_LIBS([pow], [m])
AC_SEARCH_LIBS([luaopen_utf8], [lua5.3 lua])

# Checks for header files.
AC_CHECK_HEADERS([lua5.3/lua.h lua.h])

# Checks for package versions.
AC_ARG_VAR([BINUTILS_VERSION], [manually set the version of binutils to use])
AC_ARG_VAR([GCC_VERSION], [manually set the version of gcc to use])
AC_ARG_VAR([NEWLIB_VERSION], [manually set the version of newlib to use])
AC_MSG_CHECKING([binutils version])
if [[ -z "$BINUTILS_VERSION" ]]; then
  BINUTILS_VERSION=`wget -O - -q "https://ftp.gnu.org/gnu/binutils/" | grep -E -o ">binutils-[[0-9\\\\.]]+tar\\\\.gz<" | tail -n 1 | sed -e s/\\>// -e s/\\\\.tar\\\\.gz\\<//`
fi
AC_MSG_RESULT([$BINUTILS_VERSION])
AC_MSG_CHECKING([gcc version])
if [[ -z "$GCC_VERSION" ]]; then
  GCC_VERSION=`wget -O - -q "https://ftp.gnu.org/gnu/gcc/" | grep -E -o ">gcc-[[0-9\\\\.]]+/<" | tail -n 1 | sed -e s/\\>// -e s/\\\\/\\<//`
fi
AC_MSG_RESULT([$GCC_VERSION])
AC_MSG_CHECKING([newlib version])
if [[ -z "$NEWLIB_VERSION" ]]; then
  NEWLIB_VERSION=`wget -O - -q "https://sourceware.org/pub/newlib/" | grep -E -o ">newlib-[[0-9\\\\.]]+tar\\\\.gz<" | head -n 1 | sed -e s/\\>// -e s/\\\\.tar\\\\.gz\\<//`
fi
AC_MSG_RESULT([$NEWLIB_VERSION])

AC_CONFIG_HEADERS([src/libgru/config.h])
AC_CONFIG_FILES([makefile src/gfxdis/makefile src/grc/makefile src/libgru/makefile src/libgs/makefile src/luapatch/makefile])
AC_OUTPUT
