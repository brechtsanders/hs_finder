hs_finder
=========
Cross-platform C library for finding (and replacing) multiple patterns in a stream of data.

Description
-----------
The hs_finder library is cross-platform C library that allows searching a stream of data for multiple search patterns.
The actual searching is done using the Hyperscan library.
Multiple patterns can be searched at the same time, and multiple searches can be layered.
Layered searches means that the output of the first search is presented as input for the second search, and so on.

Goal
----
The library was written with the following goals in mind:
- written in standard C, but allows being used by C++
- speed
- small footprint
- portable across different platforms (Windows, Mac, *nix)

Libraries
---------
The following libraries are provided:
- `-lhs_finder` - requires `#include <hs_finder.h>`

Command line utilities
----------------------
Some command line utilities are included:
- `hs_finder_count` - counts how much time a pattern appears
- `hs_finder_replace` - replaces patterns with other patterns

Dependancies
------------
This project has only one external depencancy:
- Hyperscan - https://www.hyperscan.io/

Building from source
--------------------
Requirements:
- a C compiler like gcc or clang, on Windows MinGW and MinGW-w64 are supported
- a shell environment, on Windows MSYS is supported
- CMake version 2.6 or higher

Building with CMake
- configure by running `cmake -G"Unix Makefiles"` (or `cmake -G"MSYS Makefiles"` on Windows) optionally followed by:
  + `-DCMAKE_INSTALL_PREFIX:PATH=<path>` Base path were files will be installed
  + `-DBUILD_STATIC:BOOL=OFF` - Don't build static libraries
  + `-DBUILD_SHARED:BOOL=OFF` - Don't build shared libraries
  + `-DBUILD_TOOLS:BOOL=OFF` - Don't build tools (only libraries)
- build and install by running `make install` (or `make install/strip` to strip symbols)

For Windows prebuilt binaries are also available for download (both 32-bit and 64-bit)

License
-------
hs_finder is released under the terms of the 3-clause BSD license, see LICENSE.txt.

This means you are free to use hs_finder in any of your projects, from open source to commercial.
