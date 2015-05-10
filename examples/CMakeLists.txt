
add_executable(testhid testhid.cpp)
target_link_libraries(testhid usbpp ${LIBUSB_LIBRARIES})

install(TARGETS testhid DESTINATION ${BINDIR})
