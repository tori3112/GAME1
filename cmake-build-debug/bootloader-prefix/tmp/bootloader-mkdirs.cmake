# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/wiktoria/esp/esp-idf/components/bootloader/subproject"
  "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader"
  "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader-prefix"
  "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader-prefix/tmp"
  "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader-prefix/src/bootloader-stamp"
  "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader-prefix/src"
  "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/wiktoria/Documents/labs/ee3/esp-idf-mirf/GAME1/cmake-build-debug/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
