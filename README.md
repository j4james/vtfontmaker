VT Font Maker
=============

![Screenshot](screenshot.png)

This is a TUI application for editing VT soft fonts, also known as Dynamically
Redefinable Character Sets. It requires a VT525 (or something of comparable
functionality) to run, but it should be capable of editing fonts from most if
not all of the DEC terminals.


Quick Start
-----------

* Use the cursor keys to move
* Hold down `Alt` while moving to select a range
* Press the `Space` bar to toggle a pixel
* Use `F10` to open the menu


Download
--------

The latest binaries can be found on GitHub at the following url:

https://github.com/j4james/vtfontmaker/releases/latest

For Linux download `vtfontmaker`, and for Windows download `vtfontmaker.exe`.


Build Instructions
------------------

If you want to build this yourself, you'll need [CMake] version 3.15 or later
and a C++ compiler supporting C++20 or later.

1. Download or clone the source:  
   `git clone https://github.com/j4james/vtfontmaker.git`

2. Change into the build directory:  
   `cd vtfontmaker/build`

3. Generate the build project:  
   `cmake -D CMAKE_BUILD_TYPE=Release ..`

4. Start the build:  
   `cmake --build . --config Release`

[CMake]: https://cmake.org/


License
-------

The VT Font Maker source code and binaries are released under the MIT License.
See the [LICENSE] file for full license details.

[LICENSE]: LICENSE.txt
