/*
 * This file is part of Usbpp, a C++ wrapper around libusb
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * Usbpp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBUSBPP_DEVICE_H_
#define LIBUSBPP_DEVICE_H_

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "buffer.h"
#include "exception.h"
#include "stddevicehash.h"

struct libusb_device_descriptor;
struct libusb_device;

namespace Usbpp {

class Context;

/**
 * An exception thrown when the device cannot be opened.
 */
class DeviceOpenException : public Exception {
public:
	explicit DeviceOpenException(int error) noexcept;
	virtual ~DeviceOpenException();

	virtual const char* what() const noexcept;
};

/**
 * An exception thrown when the transfer failed.
 */
class DeviceTransferException : public Exception {
public:
	explicit DeviceTransferException(int error) noexcept;
	virtual ~DeviceTransferException();

	virtual const char* what() const noexcept;
};

/**
 * An USB device.
 *
 * This class provides an interface to communicate with the connected USB device.
 */
class Device {
public:
	friend class Context;
	friend struct std::hash<Device>;

	/**
	 * Default constructor.
	 *
	 * The device contructed with the default constructor is not a valid device
	 * and it must not be used. However, it can be assigned a valid device,
	 * which in turn makes the device valid.
	 *
	 * To get a valid device, use the functionality provided by \a ::Usbpp::Context.
	 */
	Device();
	/**
	 * Copy constructor.
	 */
	Device(const Device& other);
	/**
	 * Move constructor.
	 */
	Device(Device&& other) noexcept;
	/**
	 * Destructor.
	 */
	~Device();

	/**
	 * Assignment operator.
	 */
	Device& operator=(const Device& other);
	/**
	 * Move assignment operator
	 */
	Device& operator=(Device&& other) noexcept;

	/**
	 * Test whether two devices are the same.
	 */
	bool operator==(const Device& other) const;
	/**
	 * Test whether two devices are the same.
	 */
	bool operator!=(const Device& other) const;

	/**
	 * Check whether the device is a valid USB device.
	 *
	 * \return true if the device is valid.
	 */
	bool isValid() const;

	/**
	 * Open the device for use.
	 *
	 * \param detachDriver auto detach kernel driver if the kernel driver is active
	 * when claiming the interface and reattach it when the interface is released.
	 */
	void open(bool detachDriver);
	/**
	 * Close the device.
	 *
	 * The device is closed only after all the Device instances that share
	 * the same device are closed.
	 *
	 * All currently claimed interfaces are released before closing the device.
	 * Note that only the interfaces claimed by the current instance are released
	 * to ensure we don't unexpectedly release an interface claimed by another
	 * \a Device object sharing the same physical device.
	 */
	void close();

	/**
	 *  Reset the device.
	 *
	 * The reset can only be performed if the device has not been claimed.
	 *
	 * \return true if the device has been reset. If false is returned, the device
	 * has been disconnected & connected, meaning that the device must be rediscovered
	 * and reopened again.
	 */
	bool reset();

	/**
	 * Clear halt on an endpoint.
	 */
	void clearHalt(unsigned char endpoint);

	/**
	 * Get libusb device descriptor.
	 *
	 * \return a libusb_device_descriptor structure containing the device descriptor.
	 *         The structure is defined in libusb.h
	 */
	libusb_device_descriptor getDescriptor();

	/**
	 *
	 * @param desc
	 * @param key
	 * @return
	 */
	std::string* getStringDescriptor(int key);

	uint8_t getBusNumber() const;

	uint8_t getDeviceAddress() const;

	static const int MAX_PORT_NUMBERS = 255;
	std::vector<uint8_t> *getPortNumbers();

	/**
	 * Get the device configuration.
	 *
	 * \return The current device configuration.
	 */
	int getConfiguration();

	/**
	 * Set the device configuration.
	 *
	 * \param bConfigurationValue Configuration to set.
	 */
	void setConfiguration(int bConfigurationValue);

	/**
	 * Claim an interface for use.
	 *
	 * \param bInterfaceNumber Interface to claim.
	 */
	void claimInterface(int bInterfaceNumber);
	/**
	 * Release the interface.
	 *
	 * The interface is released only after all Devices that claimed this
	 * interface release it.
	 *
	 * \param bInterfaceNumber Interface to release.
	 */
	void releaseInterface(int bInterfaceNumber);

	/**
	 * Control transfer from the device to the computer ("receive").
	 *
	 * \param bmRequestType The request type field for the setup packet.
	 * \param bRequest The request field for the setup packet.
	 * \param wValue The value field for the setup packet.
	 * \param wIndex The index field for the setup packet.
	 * \param data Buffer where the received data will be stored. The buffer must
	 *        be preallocated to store received data.
	 * \param timeout timeout (in millseconds) that this function should wait
	 *        before giving up due to no response being received.
	 *        For an unlimited timeout, use value 0.
	 * \return Number of bytes actually written to \a data buffer.
	 */
	int controlTransferIn(uint8_t bmRequestType,
	                      uint8_t bRequest,
	                      uint16_t wValue,
	                      uint16_t wIndex,
	                      ByteBuffer& data,
	                      unsigned int timeout) const;
	/**
	 * Bulk transfer from the device to the computer ("receive").
	 *
	 * \param endpoint The address of a valid endpoint to communicate with.
	 * \param data Buffer where the received data will be stored. The buffer must
	 *        be preallocated to the maximum expected amount of data.
	 * \param timeout timeout (in millseconds) that this function should wait
	 *        before giving up due to no response being received.
	 *        For an unlimited timeout, use value 0.
	 * \return Number of bytes actually transferred.
	 */
	int bulkTransferIn(unsigned char endpoint,
	                   ByteBuffer& data,
	                   unsigned int timeout) const;
	/**
	 * Interrupt transfer from the device to the computer ("receive").
	 *
	 * \param endpoint The address of a valid endpoint to communicate with.
	 * \param data Buffer where the received data will be stored. The buffer must
	 *        be preallocated to the maximum expected amount of data.
	 * \param timeout timeout (in millseconds) that this function should wait
	 *        before giving up due to no response being received.
	 *        For an unlimited timeout, use value 0.
	 * \return Number of bytes actually transferred.
	 */
	int interruptTransferIn(unsigned char endpoint,
	                        ByteBuffer& data,
	                        unsigned int timeout) const;

	/**
	 * Control transfer from computer to device ("send").
	 *
	 * \param bmRequestType The request type field for the setup packet.
	 * \param bRequest The request field for the setup packet.
	 * \param wValue The value field for the setup packet.
	 * \param wIndex The index field for the setup packet.
	 * \param data Buffer with data to send.
	 * \param timeout timeout (in millseconds) that this function should wait
	 *        before giving up due to no response being received.
	 *        For an unlimited timeout, use value 0.
	 * \return Number of bytes actually sent.
	 */
	int controlTransferOut(uint8_t bmRequestType,
	                       uint8_t bRequest,
	                       uint16_t wValue,
	                       uint16_t wIndex,
	                       const ByteBuffer& data,
	                       unsigned int timeout) const;
	/**
	 * Bulk transfer from computer to device ("send").
	 *
	 * \param endpoint The address of a valid endpoint to communicate with.
	 * \param data Buffer with data to send.
	 * \param timeout timeout (in millseconds) that this function should wait
	 *        before giving up due to no response being received.
	 *        For an unlimited timeout, use value 0.
	 * \return Number of bytes actually transferred.
	 */
	int bulkTransferOut(unsigned char endpoint,
	                    const ByteBuffer& data,
	                    unsigned int timeout) const;
	/**
	 * Interrupt transfer from computer to device ("send").
	 *
	 * \param endpoint The address of a valid endpoint to communicate with.
	 * \param data Buffer with data to send.
	 * \param timeout timeout (in millseconds) that this function should wait
	 *        before giving up due to no response being received.
	 *        For an unlimited timeout, use value 0.
	 * \return Number of bytes actually transferred.
	 */
	int interruptTransferOut(unsigned char endpoint,
	                         const ByteBuffer& data,
	                         unsigned int timeout) const;

private:
	explicit Device(libusb_device* device_);
	class Impl;
	std::unique_ptr<Impl> pimpl;
};

}

#endif
