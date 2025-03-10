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
AC_SUBST([GCC_IN_TREE_DEPS])
AC_ARG_ENABLE([gcc-in-tree-deps],
              [AS_HELP_STRING([--disable-gcc-in-tree-deps],
                              [do not download in-tree gcc prerequisites])],
              [], [enable_gcc_in_tree_deps=yes])
GCC_IN_TREE_DEPS="$enable_gcc_in_tree_deps"
AC_SUBST([CFLAGS_FOR_TARGET])
AC_SUBST([CXXFLAGS_FOR_TARGET])
CFLAGS_FOR_TARGET="-g -ffunction-sections -fdata-sections -mno-check-zero-division $CFLAGS_FOR_TARGET"
CXXFLAGS_FOR_TARGET="-g -ffunction-sections -fdata-sections -mno-check-zero-division $CXXFLAGS_FOR_TARGET"
AC_ARG_ENABLE([vc],
              [AS_HELP_STRING([--enable-vc],
                              [compile libraries with wii vc compatibility])],
              [
                CFLAGS_FOR_TARGET="-Os -fno-reorder-blocks -fno-optimize-sibling-calls $CFLAGS_FOR_TARGET"
                CXXFLAGS_FOR_TARGET="-Os -fno-reorder-blocks -fno-optimize-sibling-calls $CXXFLAGS_FOR_TARGET"
              ],
              [
                CFLAGS_FOR_TARGET="-Os $CFLAGS_FOR_TARGET"
                CXXFLAGS_FOR_TARGET="-Os $CXXFLAGS_FOR_TARGET"
              ])
AC_SUBST([CONFIG_TOOLCHAIN])
AC_ARG_WITH([configure-toolchain],
            [AS_HELP_STRING([--with-configure-toolchain=<params>],
                            [additional parameters to pass to toolchain configure script])],
            [CONFIG_TOOLCHAIN="$withval"])
AC_ARG_WITH([toolchain],
            [AS_HELP_STRING([--without-toolchain],
                            [do not check any toolchain package versions])],
            [], [with_toolchain=yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_CHECK_PROGS([WGET], [curl wget])
AS_IF([test "x$WGET" = xcurl], [WGET="curl -o"; WGET_Q="curl -s -o";],
      [test "x$WGET" = xwget], [WGET="wget -O"; WGET_Q="wget -q -O";])

# Checks for C compiler characteristics.
AC_C_BIGENDIAN

# Checks for libraries.
AC_SUBST([LIBM])
AC_SUBST([LIBLUA])
AC_SUBST([ZLIB])
AC_SUBST([LIBUSB])
AC_SUBST([LIBUTIL])
AC_SUBST([LIBWS2_32])
AC_SEARCH_LIBS([pow], [m],
               [AS_IF([test -n "$ac_lib"], [LIBM=-l$ac_lib])])
AC_SEARCH_LIBS([luaopen_utf8], [lua-5.4 lua5.4 lua-5.3 lua5.3 lua],
               [AS_IF([test -n "$ac_lib"], [LIBLUA=-l$ac_lib])])
AC_SEARCH_LIBS([zlibVersion], [z],
               [AS_IF([test -n "$ac_lib"], [ZLIB=-l$ac_lib])])
AC_SEARCH_LIBS([libusb_init], [usb-1.0 usb],
               [AS_IF([test -n "$ac_lib"], [LIBUSB=-l$ac_lib])],
               [
                 AC_SEARCH_LIBS([libusb_set_option], [usb-1.0 usb],
                                [AS_IF([test -n "$ac_lib"], [LIBUSB=-l$ac_lib])])
               ])
AC_SEARCH_LIBS([libusb_set_option], [usb-1.0 usb],
               [AS_IF([test -n "$ac_lib"], [LIBUSB=-l$ac_lib])])
AC_SEARCH_LIBS([openpty], [util],
               [AS_IF([test -n "$ac_lib"], [LIBUTIL=-l$ac_lib])])
AC_SEARCH_LIBS([gai_strerrorA], [Ws2_32],
               [AS_IF([test -n "$ac_lib"], [LIBWS2_32=-l$ac_lib])])

# Checks for header files.
AC_CHECK_HEADERS([lua5.4/lua.h lua54/lua.h lua5.3/lua.h lua53/lua.h lua.h])
AC_CHECK_HEADERS([zlib.h])
AC_CHECK_HEADERS([libusb-1.0/libusb.h libusb.h])
AC_CHECK_HEADERS([pty.h util.h libutil.h])

# Checks for declarations.
AC_CHECK_DECLS([asprintf])

# Checks for package versions.
AC_ARG_VAR([BINUTILS_VERSION], [manually set the version of binutils to use])
AC_ARG_VAR([GCC_VERSION], [manually set the version of gcc to use])
AC_ARG_VAR([NEWLIB_VERSION], [manually set the version of newlib to use])
AC_ARG_VAR([GDB_VERSION], [manually set the version of gdb to use])
AS_IF([test "x$with_toolchain" != xno],
      [
        AS_IF([test -z "$WGET"],
              [AC_MSG_ERROR([don't know how to check package versions, configure --without-toolchain],
                            [1])])
        AC_MSG_CHECKING([binutils version])
        AS_IF([test -z "$BINUTILS_VERSION"],
              [
                BINUTILS_VERSION=`$WGET_Q - "https://ftp.gnu.org/gnu/binutils/" |
                                  grep -E -o ">binutils-[[0-9\\\\.]]+tar\\\\.xz<" |
                                  tail -n 1 |
                                  sed -e s/\\>// -e s/\\\\.tar\\\\.xz\\<//`
              ])
        AC_MSG_RESULT([$BINUTILS_VERSION])
        AC_MSG_CHECKING([gcc version])
        AS_IF([test -z "$GCC_VERSION"],
              [
                GCC_VERSION=`$WGET_Q - "https://ftp.gnu.org/gnu/gcc/" |
                             grep -E -o ">gcc-[[0-9\\\\.]]+/<" |
                             tail -n 1 |
                             sed -e s/\\>// -e s/\\\\/\\<//`
              ])
        AC_MSG_RESULT([$GCC_VERSION])
        AC_MSG_CHECKING([newlib version])
        AS_IF([test -z "$NEWLIB_VERSION"],
              [
                NEWLIB_VERSION=`$WGET_Q - "https://sourceware.org/pub/newlib/" |
                                grep -E -o ">newlib-[[0-9\\\\.]]+tar\\\\.gz<" |
                                head -n 1 |
                                sed -e s/\\>// -e s/\\\\.tar\\\\.gz\\<//`
              ])
        AC_MSG_RESULT([$NEWLIB_VERSION])
        AC_MSG_CHECKING([gdb version])
        AS_IF([test -z "$GDB_VERSION"],
              [
                GDB_VERSION=`$WGET_Q - "https://ftp.gnu.org/gnu/gdb/" |
                             grep -E -o ">gdb-[[0-9\\\\.]]+tar\\\\.xz<" |
                             tail -n 1 |
                             sed -e s/\\>// -e s/\\\\.tar\\\\.xz\\<//`
              ])
        AC_MSG_RESULT([$GDB_VERSION])
      ])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([makefile src/ed64rdb/makefile src/gfxdis/makefile src/grc/makefile src/libgru/makefile src/libgs/makefile src/luapatch/makefile])
AC_OUTPUT
