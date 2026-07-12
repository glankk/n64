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
AC_ARG_ENABLE([toolchain-gdb],
              [AS_HELP_STRING([--disable-toolchain-gdb],
                              [do not build the optional GDB and simulator components])],
              [AS_IF([test "x$enableval" != xno],
                     [AC_MSG_ERROR([GDB is enabled by default; only --disable-toolchain-gdb is supported])])
               enable_toolchain_gdb=no],
              [enable_toolchain_gdb=yes])

# Host platform setup. The target remains mips64 on every supported host.
AC_ARG_VAR([HOMEBREW_PREFIX], [Homebrew prefix to use on macOS])
AC_SUBST([HOST_OS])
AC_SUBST([HOST_ARCH])
AC_SUBST([TOOLCHAIN_PATH])
HOST_OS=`uname -s 2>/dev/null || echo unknown`
HOST_ARCH=`uname -m 2>/dev/null || echo unknown`
TOOLCHAIN_PATH=
AC_MSG_NOTICE([host platform: $HOST_OS/$HOST_ARCH])

# GDB and the simulator are optional toolchain components.
AS_IF([test "x$enable_toolchain_gdb" = xno],
      [
        case " $CONFIG_TOOLCHAIN " in
          *" --disable-gdb"*) ;;
          *) CONFIG_TOOLCHAIN="$CONFIG_TOOLCHAIN --disable-gdb --disable-sim" ;;
        esac
      ])
AC_MSG_NOTICE([toolchain GDB component: $enable_toolchain_gdb])

# Apple Silicon Homebrew installs outside the compiler's default search path.
# Add both the global prefix and formula-specific prefixes (including keg-only
# formulae such as zlib and texinfo). These flags are only added on Darwin and
# therefore do not alter Linux, BSD, or MSYS2 builds.
AS_IF([test "x$HOST_OS" = xDarwin],
      [
        AS_IF([test "x$HOST_ARCH" = xarm64],
              [case " $CONFIG_TOOLCHAIN " in
                 *" --disable-werror"*) ;;
                 *) CONFIG_TOOLCHAIN="$CONFIG_TOOLCHAIN --disable-werror" ;;
               esac])

        AC_MSG_CHECKING([for Homebrew])
        n64_brew=
        for n64_brew_candidate in brew /opt/homebrew/bin/brew /usr/local/bin/brew; do
          if test "x$n64_brew_candidate" = xbrew; then
            n64_brew_candidate=`command -v brew 2>/dev/null || true`
          fi
          if test -n "$n64_brew_candidate" && test -x "$n64_brew_candidate"; then
            n64_brew="$n64_brew_candidate"
            break
          fi
        done
        if test -z "$HOMEBREW_PREFIX" && test -n "$n64_brew"; then
          HOMEBREW_PREFIX=`"$n64_brew" --prefix 2>/dev/null || true`
        fi
        AS_IF([test -n "$HOMEBREW_PREFIX"],
              [AC_MSG_RESULT([$HOMEBREW_PREFIX])],
              [AC_MSG_RESULT([not found])])

        n64_gmp_prefix=
        n64_mpfr_prefix=
        n64_mpc_prefix=
        if test -n "$HOMEBREW_PREFIX"; then
          test -d "$HOMEBREW_PREFIX/include" && CPPFLAGS="$CPPFLAGS -I$HOMEBREW_PREFIX/include"
          test -d "$HOMEBREW_PREFIX/lib" && LDFLAGS="$LDFLAGS -L$HOMEBREW_PREFIX/lib"

          for n64_formula in lua zlib jansson libusb gmp mpfr libmpc; do
            n64_formula_prefix=
            if test -n "$n64_brew"; then
              n64_formula_prefix=`"$n64_brew" --prefix "$n64_formula" 2>/dev/null || true`
            elif test -d "$HOMEBREW_PREFIX/opt/$n64_formula"; then
              n64_formula_prefix="$HOMEBREW_PREFIX/opt/$n64_formula"
            fi
            if test -n "$n64_formula_prefix"; then
              test -d "$n64_formula_prefix/include" && CPPFLAGS="$CPPFLAGS -I$n64_formula_prefix/include"
              test -d "$n64_formula_prefix/lib" && LDFLAGS="$LDFLAGS -L$n64_formula_prefix/lib"
              case "$n64_formula" in
                gmp) n64_gmp_prefix="$n64_formula_prefix" ;;
                mpfr) n64_mpfr_prefix="$n64_formula_prefix" ;;
                libmpc) n64_mpc_prefix="$n64_formula_prefix" ;;
              esac
            fi
          done

          # GCC's libgcc generators rely on GNU sed regular-expression
          # behaviour. Homebrew installs it keg-only, so expose its unprefixed
          # tools to the recursive toolchain build on macOS only.
          if test -d "$HOMEBREW_PREFIX/opt/gnu-sed/libexec/gnubin"; then
            TOOLCHAIN_PATH="$HOMEBREW_PREFIX/opt/gnu-sed/libexec/gnubin"
          fi

          # Homebrew's texinfo is keg-only. Export its tools to every recursive
          # toolchain build through the generated Makefile.
          if test -d "$HOMEBREW_PREFIX/opt/texinfo/bin"; then
            if test -n "$TOOLCHAIN_PATH"; then
              TOOLCHAIN_PATH="$TOOLCHAIN_PATH:$HOMEBREW_PREFIX/opt/texinfo/bin"
            else
              TOOLCHAIN_PATH="$HOMEBREW_PREFIX/opt/texinfo/bin"
            fi
          fi
          if test -d "$HOMEBREW_PREFIX/bin"; then
            if test -n "$TOOLCHAIN_PATH"; then
              TOOLCHAIN_PATH="$TOOLCHAIN_PATH:$HOMEBREW_PREFIX/bin"
            else
              TOOLCHAIN_PATH="$HOMEBREW_PREFIX/bin"
            fi
          fi

          # Pass Homebrew's prerequisite prefixes to the recursive GNU
          # toolchain configure unless the user supplied an override.
          if test -n "$n64_gmp_prefix"; then
            case " $CONFIG_TOOLCHAIN " in
              *" --with-gmp="*) ;;
              *) CONFIG_TOOLCHAIN="$CONFIG_TOOLCHAIN --with-gmp=$n64_gmp_prefix" ;;
            esac
          fi
          if test -n "$n64_mpfr_prefix"; then
            case " $CONFIG_TOOLCHAIN " in
              *" --with-mpfr="*) ;;
              *) CONFIG_TOOLCHAIN="$CONFIG_TOOLCHAIN --with-mpfr=$n64_mpfr_prefix" ;;
            esac
          fi
          if test -n "$n64_mpc_prefix"; then
            case " $CONFIG_TOOLCHAIN " in
              *" --with-mpc="*) ;;
              *) CONFIG_TOOLCHAIN="$CONFIG_TOOLCHAIN --with-mpc=$n64_mpc_prefix" ;;
            esac
          fi
        fi
      ])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_CHECK_TOOL([STRIP], [strip], :)
AC_SUBST([INSTALL_STRIP])
AC_MSG_CHECKING([install -s])
AS_IF([$INSTALL --strip-program=$STRIP -d . 2>/dev/null],
      [INSTALL_STRIP="$INSTALL --strip-program=$STRIP -s"],
      [INSTALL_STRIP="STRIPPROG=$STRIP $INSTALL -s"])
AC_MSG_RESULT([$INSTALL_STRIP])
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
AC_CHECK_HEADERS([lua5.4/lua.h lua54/lua.h lua5.3/lua.h lua53/lua.h lua/lua.h lua.h])
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
AC_ARG_VAR([N64_TOOLCHAIN_ARCHIVE_DIR], [directory containing local binutils, GCC, newlib, and GDB archives])
AC_ARG_VAR([BINUTILS_ARCHIVE], [local binutils .tar.xz archive])
AC_ARG_VAR([GCC_ARCHIVE], [local GCC .tar.xz archive])
AC_ARG_VAR([NEWLIB_ARCHIVE], [local newlib .tar.gz archive])
AC_ARG_VAR([GDB_ARCHIVE], [local GDB .tar.xz archive])

# Local archives are optional. Existing network URLs remain the default, but
# infer archive stems when local files were selected so offline builds do not
# need to scrape the upstream directory listings.
AS_IF([test -z "$BINUTILS_VERSION" && test -n "$BINUTILS_ARCHIVE"],
      [BINUTILS_VERSION=`basename "$BINUTILS_ARCHIVE" .tar.xz`])
AS_IF([test -z "$GCC_VERSION" && test -n "$GCC_ARCHIVE"],
      [GCC_VERSION=`basename "$GCC_ARCHIVE" .tar.xz`])
AS_IF([test -z "$NEWLIB_VERSION" && test -n "$NEWLIB_ARCHIVE"],
      [NEWLIB_VERSION=`basename "$NEWLIB_ARCHIVE" .tar.gz`])
AS_IF([test -z "$GDB_VERSION" && test -n "$GDB_ARCHIVE"],
      [GDB_VERSION=`basename "$GDB_ARCHIVE" .tar.xz`])

AS_IF([test -n "$N64_TOOLCHAIN_ARCHIVE_DIR"],
      [
        AS_IF([test -z "$BINUTILS_VERSION"],
              [BINUTILS_VERSION=`for f in "$N64_TOOLCHAIN_ARCHIVE_DIR"/binutils-*.tar.xz; do test -f "$f" && basename "$f" .tar.xz; done | "$srcdir/config/version-sort" | tail -n 1`])
        AS_IF([test -z "$GCC_VERSION"],
              [GCC_VERSION=`for f in "$N64_TOOLCHAIN_ARCHIVE_DIR"/gcc-*.tar.xz; do test -f "$f" && basename "$f" .tar.xz; done | "$srcdir/config/version-sort" | tail -n 1`])
        AS_IF([test -z "$NEWLIB_VERSION"],
              [NEWLIB_VERSION=`for f in "$N64_TOOLCHAIN_ARCHIVE_DIR"/newlib-*.tar.gz; do test -f "$f" && basename "$f" .tar.gz; done | "$srcdir/config/version-sort" | tail -n 1`])
        AS_IF([test -z "$GDB_VERSION"],
              [GDB_VERSION=`for f in "$N64_TOOLCHAIN_ARCHIVE_DIR"/gdb-*.tar.xz; do test -f "$f" && basename "$f" .tar.xz; done | "$srcdir/config/version-sort" | tail -n 1`])
      ])
AS_IF([test "x$with_toolchain" != xno],
      [
        AS_IF([test -z "$WGET" &&
               (test -z "$BINUTILS_VERSION" || test -z "$GCC_VERSION" ||
                test -z "$NEWLIB_VERSION" || test -z "$GDB_VERSION")],
              [AC_MSG_ERROR([don't know how to check package versions; provide local archives and version stems, or configure --without-toolchain],
                            [1])])
        AC_MSG_CHECKING([binutils version])
        AS_IF([test -z "$BINUTILS_VERSION"],
              [
                BINUTILS_VERSION=`$WGET_Q - "https://ftp.gnu.org/gnu/binutils/" |
                                  grep -E -o ">binutils-[[0-9\\\\.]]+tar\\\\.xz<" |
                                  sed -e s/\\>// -e s/\\\\.tar\\\\.xz\\<// |
                                  "$srcdir/config/version-sort" | tail -n 1`
              ])
        AC_MSG_RESULT([$BINUTILS_VERSION])
        AC_MSG_CHECKING([gcc version])
        AS_IF([test -z "$GCC_VERSION"],
              [
                GCC_VERSION=`$WGET_Q - "https://ftp.gnu.org/gnu/gcc/" |
                             grep -E -o ">gcc-[[0-9\\\\.]]+/<" |
                             sed -e s/\\>// -e s/\\\\/\\<// |
                             "$srcdir/config/version-sort" | tail -n 1`
              ])
        AC_MSG_RESULT([$GCC_VERSION])
        AC_MSG_CHECKING([newlib version])
        AS_IF([test -z "$NEWLIB_VERSION"],
              [
                NEWLIB_VERSION=`$WGET_Q - "https://sourceware.org/pub/newlib/" |
                                grep -E -o ">newlib-[[0-9\\\\.]]+tar\\\\.gz<" |
                                sed -e s/\\>// -e s/\\\\.tar\\\\.gz\\<// |
                                "$srcdir/config/version-sort" | tail -n 1`
              ])
        AC_MSG_RESULT([$NEWLIB_VERSION])
        AC_MSG_CHECKING([gdb version])
        AS_IF([test -z "$GDB_VERSION"],
              [
                GDB_VERSION=`$WGET_Q - "https://ftp.gnu.org/gnu/gdb/" |
                             grep -E -o ">gdb-[[0-9\\\\.]]+tar\\\\.xz<" |
                             sed -e s/\\>// -e s/\\\\.tar\\\\.xz\\<// |
                             "$srcdir/config/version-sort" | tail -n 1`
              ])
        AC_MSG_RESULT([$GDB_VERSION])
        AS_IF([test -z "$BINUTILS_VERSION" || test -z "$GCC_VERSION" ||
               test -z "$NEWLIB_VERSION" || test -z "$GDB_VERSION"],
              [AC_MSG_ERROR([failed to determine one or more toolchain versions; set BINUTILS_VERSION, GCC_VERSION, NEWLIB_VERSION, and GDB_VERSION explicitly])])
      ])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/ed64rdb/Makefile src/gfxdis/Makefile src/grc/Makefile src/libgru/Makefile src/libgs/Makefile src/luapatch/Makefile])
AC_OUTPUT
