## About
This is a collection of files and tools used to compile and test code for the
n64.

## Toolchain
`toolchain-install` provides `binutils`, `gcc`, `newlib`, and `gdb` targeting
mips64, with `-march=vr4300` and `-mabi=32` as defaults for `gcc`.

## Programs
-   `ed64rdb`: ED64v3 USB packet translator, for remote debugging of gz with
    gdb.
-   `gfxdis`: F3DEX2 display list disassembler / decompiler.
-   `grc`: Resource compiler for creating linkable n64 texture objects.
-   `gru`: Lua environment for manipulating raw binaries, n64 roms, zelda64
    file systems, ups patches, and gameshark codes.
-   `gs`: Gameshark upload tool for use with a Moschip USB Parallel Cable.
-   `luapatch`: Tool to generate lua sources from binaries, to inject into
    emulated RAM in BizHawk.

## Headers
-   `list/list.c/h`: Doubly linked list implementation.
-   `memory/memory.c/h`: Minimal malloc implementation.
-   `set/set.c/h`: Ordered set implementation.
-   `vector/vector.c/h`: Vector implementation.
-   `grc.c/h`: API for resources created by `grc`.
-   `mips.h`: VR4300 mips3 header, with convenience definitions and assembler
    macros.
-   `n64.h`: Mostly-compatible definitions for a subset of the ultra library.
-   `startup.c/h`: Basic program initialization routines.
-   `vr4300.c/h`: Disassembler library.

## Libraries
-   `gl-n64.ld`: Linker script for n64 binaries.

## Prerequisites
You'll need to have these packages installed: `wget tar make diffutils texinfo
gcc g++ lua5.3-or-5.4 zlib jansson libusb-1.0 libgmp libmpfr libmpc`. See
Installation, step 3. If you
want to use the included `luapatch` program with BizHawk, you should set the
`EMUDIR` environment variable to the path to your BizHawk installation.

## Installation
### Prebuilt packages
Binary packages are available, see https://github.com/PracticeROM/packages.
Packages are built from the `n64-ultra` branch and target `mips64-ultra-elf`

### macOS on Apple Silicon
The build supports native `arm64` hosts.
Install Apple's command-line tools 
and Homebrew first, then run:

```sh
xcode-select --install  # skip this when already installed
./install_deps
./configure --prefix="$HOME/.local/n64"
gmake toolchain-all
gmake toolchain-install
gmake
gmake install
gmake install-sys
export PATH="$HOME/.local/n64/bin:$PATH"
```

`configure` automatically detects Homebrew in `/opt/homebrew` (Apple Silicon)
or `/usr/local` (Intel), adds formula-specific include/library paths, passes the
installed GMP/MPFR/MPC locations to the combined GCC/GDB build, and exposes the
keg-only `texinfo` tools to recursive builds. Homebrew's `gmake` is used for
the toolchain build. `install_deps` also installs GNU sed and `configure` exposes its `gnubin` directory only to recursive toolchain builds; the host shell does not need a permanent PATH change. A user-writable prefix is recommended; using `/opt/n64` may require
administrator privileges during installation.

The toolchain package versions are selected by the existing version checks. To
reproduce a build with exact versions, set the full archive stem before running
`configure`, for example:

```sh
BINUTILS_VERSION=binutils-X.Y \
GCC_VERSION=gcc-X.Y.Z \
NEWLIB_VERSION=newlib-X.Y.Z.YYYYMMDD \
GDB_VERSION=gdb-X.Y \
./configure --prefix="$HOME/.local/n64"
```

The included newlib patches continue to be applied conditionally, so older
newlib releases and `newlib-4.6.0.20260123` or later remain supported without
forcing a version change.

GDB is optional for OoTR. It is disabled by default only on Apple Silicon to
avoid incompatibilities between independently selected current GDB and
binutils releases. Use `--enable-toolchain-gdb` when GDB is explicitly needed.

The existing GNU and Sourceware download URLs remain the default. To use local
archives instead, put files named like `binutils-X.Y.tar.xz`,
`gcc-X.Y.Z.tar.xz`, `newlib-X.Y.Z.YYYYMMDD.tar.gz`, and `gdb-X.Y.tar.xz` in one
directory and configure with:

```sh
N64_TOOLCHAIN_ARCHIVE_DIR=/absolute/path/to/archives \
./configure --prefix="$HOME/.local/n64"
```

Individual files can instead be selected with `BINUTILS_ARCHIVE`,
`GCC_ARCHIVE`, `NEWLIB_ARCHIVE`, and `GDB_ARCHIVE`. When building completely
offline, either provide a GCC source tree that already contains its prerequisite
sources or add `--disable-gcc-in-tree-deps` and use installed GMP/MPFR/MPC.

### Building from source
1.  If you're on Windows, download and install MSYS2 from
    [here](https://msys2.github.io/). Open a `MSYS2 MinGW 64-bit` shell.
2.  Clone the repository and navigate into it by running
    `git clone https://github.com/glankk/n64 && cd n64`.
3.  Optionally, have the included script try to install the prerequisite
    packages automatically by running `./install_deps`. On macOS this uses
    Homebrew without root; system package managers on other platforms may need
    administrator privileges.
4.  Configure the package with `./configure --prefix=/opt/n64`. You may use
    another installation prefix if you wish. To enable Wii VC compatibility,
    append ` --enable-vc` to the configure command. To produce portable Windows
    executables (that don't need additional dynamic libraries) under MSYS2,
    append ` --enable-static-executables` to the configure command.
5.  Install the GNU Toolchain for MIPS by running `make toolchain-all && make
    toolchain-install`.
6.  Compile and install the included programs by running `make && make
    install`.
7.  Install the included headers and libraries by running `make install-sys`.
8.  The n64 tools and MIPS toolchain will have been installed to `/opt/n64` (or
    the prefix you chose). Add `/opt/n64/bin` to your `PATH` variable (or
    `your-prefix/bin`, usually `C:\msys64\opt\n64\bin` on Windows). If you're
    using MSYS2, also add the path to the directory containing the `make`
    program (typically `C:\msys64\usr\bin`).
