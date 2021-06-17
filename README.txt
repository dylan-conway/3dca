
-------- 3D Cellular Automata Simulator --------


### Controls

Left/right arrow  - rotate cube
z/x               - switch color mode
c                 - clear grid
f                 - fill grid
r                 - randomize grid
a/s               - switch CA rule
space             - pause/play CA animation
1                 - 1x1x1 centered cube
2                 - 4x4x4 centered cube
3                 - 8x8x8 centered cube
4                 - 6x6x6 centered randomized cube
middle mouse      - hold to rotate cube
mouse wheel       - adjust size of cube
right mouse       - delete cells
left mouse        - draw cells


### Setup

1)  mingw

  - Download installer: https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/
  - Run installer to install files in directory
  - Put the mingw64/bin and mingw64/x86_64-w64-mingw32/bin in path


2)  glew

  - Download glew source zip: https://github.com/nigels-com/glew/releases/tag/glew-2.2.0
  - Open terminal in extracted source code directory and go to /build/cmake/
  - Run: cmake -G "MinGW Makefiles" -S . -B . -DCMAKE_INSTALL_PREFIX=C:\path\to\mingw64\x86_64-w64-mingw32
  - Run: mingw32-make all
  - Run: mingw32-make install


3)  sdl2

  - Download sdl2 development libraries for windows: https://www.libsdl.org/release/SDL2-devel-2.0.14-mingw.tar.gz
  - Extract the files, open \path\to\SDL2-2.0.14\x86_64-w64-mingw32\
  - Copy directories: bin, include, lib, share
  - Paste them into mingw directory
  

4)  cglm (glm for c)

  - Get the latest release of the library: https://github.com/recp/cglm/releases
  - Unzip the files and make a build directory in it
  - cd into the build directory and run: cmake -G "MinGW Makefiles" .. -DCMAKE_INSTALL_PREFIX=C:\path\to\mingw64
  - Run: make
  - Run: make install
