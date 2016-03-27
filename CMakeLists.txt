cmake_minimum_required(VERSION 2.8.4)
project(usbpp CXX)

if (NOT BINDIR)
	set(BINDIR  "bin")
endif()
if (NOT LIBDIR)
	set(LIBDIR  "lib")
endif()
if (NOT INCDIR)
	set(INCDIR  "include/libusbpp")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.0)
find_package(Threads)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wstrict-aliasing -Wextra")
include_directories(${LIBUSB_INCLUDE_DIRS} include)

add_subdirectory(examples)
add_subdirectory(include)
add_subdirectory(src)
