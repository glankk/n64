## About
This is a collection of files and tools used to compile and test code for the
n64.

## Toolchain
`install-toolchain` provides `binutils`, `gcc`, `newlib`, and `gdb` targeting
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
gcc g++ lua5.3 jansson libusb-1.0 libgmp`. See Installation, step 4. If you
want to use the included `luapatch` program with BizHawk, you should set the
`EMUDIR` environment variable to the path to your BizHawk installation.

## Installation
### Prebuilt Package (WSL or Debian-based Linux systems)
1.  From the command line run the following to fetch the package:  
    ```
    sudo sh -c '(curl https://practicerom.com/public/packages/debian/pgp.pub || wget -O - https://practicerom.com/public/packages/debian/pgp.pub) | apt-key add - && echo deb http://practicerom.com/public/packages/debian staging main >/etc/apt/sources.list.d/practicerom.list && apt update'
    ```

2.  Install the package by running `sudo apt install n64-ultra`, or install all
    practicerom development packages by running
    `sudo apt install practicerom-dev`. The prebuilt packages are built from
    the `n64-ultra` branch and target `mips64-ultra-elf`.

### Building From Source
1.  If you're on Windows, download and install MSYS2 from
    [here](https://msys2.github.io/). Open a `MSYS2 MinGW 64-bit` shell.
2.  Clone the repository and navigate into it by running
    `git clone https://github.com/glankk/n64 && cd n64`.
3.  Optionally, have the included script try to install the prerequisite
    packages automatically by running `./install_deps`. You'll need root
    privileges for this.
4.  Configure the package with `./configure --prefix=/opt/n64`. You may use
    another installation prefix if you wish. To enable Wii VC compatibility,
    append ` --enable-vc` to the configure command. To produce portable Windows
    executables (that don't need additional dynamic libraries) under MSYS2,
    append ` --enable-static-executables` to the configure command.
5.  Install the GNU Toolchain for MIPS by running `make all-toolchain && make
    install-toolchain`. You may want to use make's `-j` option speed up this
    process.
6.  Compile and install the included programs by running `make && make
    install`.
7.  Install the included headers and libraries by running `make install-sys`.
8.  The n64 tools and MIPS toolchain will have been installed to `/opt/n64` (or
    the prefix you chose). Add `/opt/n64/bin` to your `PATH` variable (or
    `your-prefix/bin`, `C:\msys64\opt\n64\bin` on Windows). If you're using
    MSYS2, also add the path to the directory containing the `make` program
    (typically `C:\msys64\usr\bin`).
