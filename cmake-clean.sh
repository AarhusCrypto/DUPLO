#!/bin/sh
make clean
rm cmake_install.cmake
rm CMakeCache.txt
rm Makefile
rm -rf CMakeFiles

rm src/cmake_install.cmake
rm src/Makefile
rm -rf src/CMakeFiles

rm test/cmake_install.cmake
rm test/Makefile
rm -rf test/CMakeFiles
rm -rf build