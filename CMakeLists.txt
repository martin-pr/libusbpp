cmake_minimum_required(VERSION 2.8.4)
project(usbpp C CXX)

if (NOT BINDIR)
	set(BINDIR  "bin")
endif()
if (NOT LIBDIR)
	set(LIBDIR  "lib")
endif()
if (NOT INCDIR)
	set(INCDIR  "include/libusbpp")
endif()
if (NOT INSTALL_PKGCONFIG_DIR)
	set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.16)
find_package(Threads)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wstrict-aliasing -Wextra")
include_directories(${LIBUSB_INCLUDE_DIRS} include)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(examples)
add_subdirectory(include)
add_subdirectory(src)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/usbpp.pc.in ${CMAKE_CURRENT_BINARY_DIR}/usbpp.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/usbpp.pc DESTINATION "${INSTALL_PKGCONFIG_DIR}")
