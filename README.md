## About
This is a collection of files and tools used to compile and test code for the n64.

## Building the GNU toolchain for MIPS
1.  Download and unpack the [n64 repository](https://github.com/glankk/n64/archive/master.zip).
2.  Download and install MSYS2 from [here](https://msys2.github.io/).
    Open an MSYS2 shell and type `update-core`. Restart MSYS2 and type `pacman -Suu`,
    repeat until there are no more updates available.
3.  Install the packages required to build the toolchain by typing `pacman -S mingw-w64-i686-gcc tar make diffutils texinfo`.
4.  Download the latest [GCC](https://gcc.gnu.org/), [GNU Binutils](https://www.gnu.org/software/binutils/), and [Newlib](https://sourceware.org/newlib/).
    Put them in `n64-master/scripts/toolchain/`.
5.  Open a MinGW-w64 Win32 shell by starting the mingw32 application in the MSYS2 root,
    and navigate to `n64-master/scripts/toolchain/n64-toolchain` or `n64-master/scripts/toolchain/n64-toolchain-slim`.
    Run the script located in this directory by typing `./build_n64_toolchain` or `./build_n64_toolchain_slim`.

## Using Code::Blocks as IDE with the GNU toolchain and BizHawk
1.  Get the latest BizHawk release from [here](http://tasvideos.org/BizHawk.html).
2.  Get the latest Code::Blocks release from [here](http://www.codeblocks.org/).
3.  Open Code::Blocks Share Config from the Start Menu or from the installation directory.
    Select `n64-master/cb/n64.conf` as source, and `%AppData%\CodeBlocks\default.conf` as destination.
    If the destination file does not exist, open and close Code::Blocks to have it generate a default
    configuration file.
    Check all nodes except `<compiler>` from the source file and click Transfer, then click Save.
4.  Open Code::Blocks and go to Settings->Compiler. Under Selected compiler, select n64 or n64-slim.
    Under the Toolchain executables tab, change Compiler's installation directory to `msys\opt\n64-toolchain`
    or `msys\opt\n64-toolchain-slim` and add `msys\usr\bin` to Additional Paths, where msys is the
    root directory of your MSYS2 installation. Change the Program Files to the following:
    - C compiler: `mips64-gcc.exe`
    - C++ compiler: `mips64-g++.exe`
    - Linker for dynamic libs: `mips64-g++.exe`
    - Linker for static libs: `mips64-ar.exe`
    - Make program: `make.exe`

    Under the Custom variables tab, change `N64ROOT` to the path of the n64 repository,
    and `EMUDIR` to the path of your BizHawk installation.
5.  Copy `n64-master/lua/patch.lua` to the Lua subfolder of your BizHawk installation.
