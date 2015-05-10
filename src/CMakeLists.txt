
add_library(usbpp
	context.cpp device.cpp # basic libusb wrapper
	hiddevice.cpp hidreport.cpp # HID support
	mscbw.cpp mscsw.cpp msdevice.cpp msscsiinquiry.cpp msscsiinquiryresponse.cpp # mass storage
)
target_link_libraries(usbpp ${LIBUSB_LIBRARIES})

install(TARGETS usbpp DESTINATION ${LIBDIR})
