
add_library(usbpp SHARED
	buffer.cpp context.cpp device.cpp exception.cpp # basic libusb wrapper
	stddevicehash.cpp # std library support
	hiddevice.cpp hidreport.cpp # HID support
	mscbw.cpp mscsw.cpp msdevice.cpp msscsiinquiry.cpp msscsiinquiryresponse.cpp # mass storage
)
find_package(libusb-1.0)
target_link_libraries(usbpp ${LIBUSB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS usbpp DESTINATION ${LIBDIR})
