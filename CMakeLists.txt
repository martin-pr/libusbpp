cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0079 NEW)
project(usbpp C CXX)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
LINK_DIRECTORIES(/usr/local/lib /usr/local/opt)


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
find_package(libusb-1.0)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.16)
find_package(Threads)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wstrict-aliasing -Wextra")
include_directories(SYSTEM ${LIBUSB_1_INCLUDE_DIRS})
include_directories(include)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(examples)
add_subdirectory(include)
add_subdirectory(src)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/usbpp-config.cmake"
		VERSION 1.0
		COMPATIBILITY AnyNewerVersion
)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/usbpp.pc.in ${CMAKE_CURRENT_BINARY_DIR}/usbpp.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/usbpp.pc DESTINATION "${INSTALL_PKGCONFIG_DIR}")
