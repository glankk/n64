## About
This is a collection of files and tools used to compile and test code for the n64.

## Prerequisites
-  You'll need to have these packages installed: `wget tar make diffutils texinfo gcc g++ lua5.3 jansson libusb-1.0`.
   See Installation, step 4.
-  If you want to use the included `luapatch` program with BizHawk,
   you should set the `EMUDIR` environment variable to the directory of your BizHawk installation.

## Installation
1.  Download and unpack the [n64 repository](https://github.com/glankk/n64/archive/master.zip).
2.  If you're on Windows, download and install MSYS2 from [here](https://msys2.github.io/).
3.  Open a GNU/Linux or MSYS2 MinGW 32-bit shell and navigate to `n64-master`.
4.  Optionally, have the included script try to install the prerequisite packages automatically by running `scripts/install_deps`.
    You'll need root privileges for this.
5.  Install the GNU Toolchain for MIPS by running `scripts/install_toolchain`.
6.  Compile and install the n64 tools by running
    ```
    ./configure
    make && make install
    ```
7.  The n64 tools and MIPS toolchain will have been installed to `/opt/n64`.
    Add `/opt/n64/bin` to your `PATH` variable.
    If you're using MSYS2, also add the path to the `make` program (typically `C:\msys64\usr\bin`).
