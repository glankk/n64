## n64-ultra
This is the experimental branch of the n64 toolchain repository. It targets
`mips64-ultra-elf` and includes some patches to GNU toolchain;
-   GCC: Multilib configuration for targeting Wii VC, which requires lower
    optimization settings, with `-n64-wiivc`, and for disabling small-data
    sections and `$gp` accesses with `-n64-dynamic`.
-   LD: `--force-dynamic` option, used for experimenting with shared libraries.
-   GDB: Remote protocol extension to the `T` stop reply which allows the
    `library` stop reason to directly include information about which library
    was loaded or unloaded, and where, to reduce the amount of data and the
    number of round trips required for the stub to notify gdb about library
    changes.

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
gcc g++ lua5.3 jansson libusb-1.0 libgmp`. See Installation, step 4. If you
want to use the included `luapatch` program with BizHawk, you should set the
`EMUDIR` environment variable to the path to your BizHawk installation.

## Installation
### Prebuilt packages
Linux packages are available for Debian-based systems and Arch Linux. The
prebuilt packages are built from the `n64-ultra` branch and target
`mips64-ultra-elf`. This repository is split into these packages:
-   `mips64-ultra-elf-binutils`
-   `mips64-ultra-elf-gcc`
-   `mips64-ultra-elf-gcc-libs`
-   `mips64-ultra-elf-newlib`
-   `mips64-ultra-elf-gdb`
-   `mips64-ultra-elf-practicerom-libs`
-   `practicerom-tools`

The package sources are available
[here](https://github.com/PracticeROM/packages).

#### Debian, Ubuntu and WSL
1.  Run the following to install the package repository:
    ```
    sudo sh -s << EOF
        (
            curl -o /etc/apt/trusted.gpg.d/practicerom.gpg https://practicerom.com/public/packages/practicerom.gpg ||
            wget -O /etc/apt/trusted.gpg.d/practicerom.gpg https://practicerom.com/public/packages/practicerom.gpg
        ) &&
        echo 'deb [arch=amd64] http://practicerom.com/public/packages/debian unstable main' > /etc/apt/sources.list.d/practicerom.list &&
        apt update
    EOF
    ```

2.  Install indidiual packages by running e.g.
    `sudo apt install mips64-ultra-elf-gcc`, or install all practicerom
    development packages with the `practicerom-dev` metapackge:
    `sudo apt install practicerom-dev`.

#### Arch Linux
1.  Run the following to install the package repository:
    ```
    sudo sh -s << EOF
        pacman-key --init &&
        (
            curl https://practicerom.com/public/packages/practicerom.gpg ||
            wget -O - https://practicerom.com/public/packages/practicerom.gpg
        ) |
        tee -p >(pacman-key --add - >/dev/null) |
        gpg --with-colons --with-fingerprint --show-key - |
        awk -F: '\$1=="fpr" {print(\$10)}' | head -n 1 |
        xargs pacman-key --lsign &&
        (
            grep -F '[practicerom]' /etc/pacman.conf ||
            printf '\n[practicerom]\nServer = http://practicerom.com/public/packages/archlinux\n' >> /etc/pacman.conf
        ) &&
        pacman -Sy
    EOF
    ```

2.  Install indidiual packages with e.g. `sudo pacman -S mips64-ultra-elf-gcc`,
    or select practicerom development packages from the `practicerom-dev`
    group: `sudo pacman -S practicerom-dev`.

### Building from source
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
