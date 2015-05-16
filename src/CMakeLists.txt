
add_library(usbpp
	buffer.cpp context.cpp device.cpp exception.cpp # basic libusb wrapper
	hiddevice.cpp hidreport.cpp # HID support
	mscbw.cpp mscsw.cpp msdevice.cpp msscsiinquiry.cpp msscsiinquiryresponse.cpp # mass storage
)
target_link_libraries(usbpp ${LIBUSB_LIBRARIES})

install(TARGETS usbpp DESTINATION ${LIBDIR})
