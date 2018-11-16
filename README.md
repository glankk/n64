## About
This is a collection of files and tools used to compile and test code for the
n64.

## Prerequisites
You'll need to have these packages installed: `wget tar make diffutils texinfo
gcc g++ lua5.3 jansson libusb-1.0`. See Installation, step 4. If you want to
use the included `luapatch` program with BizHawk, you should set the `EMUDIR`
environment variable to the path to your BizHawk installation.

## Installation
1.  Download and unpack the
    [n64 repository](https://github.com/glankk/n64/archive/master.zip).
2.  If you're on Windows, download and install MSYS2 from
    [here](https://msys2.github.io/).
3.  Open a GNU/Linux or MSYS2 MinGW 32-bit shell and navigate to `n64-master`.
4.  Optionally, have the included script try to install the prerequisite
    packages automatically by running `./install_deps`. You'll need root
    privileges for this.
5.  Configure the package with `./configure --prefix=/opt/n64`. You may use
    another installation prefix if you wish. To enable Wii VC compatibility,
    append ` --enable-vc` to the configure command. To produce portable Windows
    executables (that don't need additional dynamic libraries) under MSYS2,
    append ` --enable-static-executables` to the configure command.
5.  Install the GNU Toolchain for MIPS by running `make install-toolchain`.
6.  Compile and install the n64 tools by running `make && make install`.
7.  Install the included headers and libraries by running `make install-sys`.
8.  The n64 tools and MIPS toolchain will have been installed to `/opt/n64` (or
    the prefix you chose). Add `/opt/n64/bin` to your `PATH` variable (or
    `your-prefix/bin`, `C:\msys64\opt\n64\bin` on Windows). If you're using
    MSYS2, also add the path to the directory containing the `make` program
    (typically `C:\msys64\usr\bin`).
