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

#ifndef LIBUSBPP_CONTEXT_H_
#define LIBUSBPP_CONTEXT_H_

#include <functional>
#include <memory>
#include <vector>

#include "device.h"
#include "exception.h"

namespace Usbpp {

/**
 * An exception thrown when context cannot be initialized.
 */
class ContextInitException : public Exception {
public:
	explicit ContextInitException(int error) noexcept;
	virtual ~ContextInitException();

	virtual const char* what() const noexcept;
};

/**
 * An exception thrown when it's not possible to obtain device list.
 */
class ContextEnumerateException : public Exception {
public:
	explicit ContextEnumerateException(int error) noexcept;
	virtual ~ContextEnumerateException();

	virtual const char* what() const noexcept;
};

/**
 * An exception thrown when it's not possible to register callbacks.
 */
class ContextRegisterCBException : public Exception {
public:
	explicit ContextRegisterCBException(int error) noexcept;
	virtual ~ContextRegisterCBException();

	virtual const char* what() const noexcept;
};


/**
 * A context.
 *
 * Handles a libusb context for an individual session.
 */
class Context {
public:
	/**
	 * Constructs a new context.
	 */
	Context();
	Context(const Context &other);
	Context(Context &&other) noexcept;
	~Context();

	Context &operator=(const Context &other);
	Context &operator=(Context &&other) noexcept;

	/**
	 * Get list of USB devices.
	 *
	 * \return list of USB devices
	 */
	std::vector<Device> getDevices();

	/**
	 * Register a function that is called when a new device is connected.
	 *
	 * The function is called asynchronously from a context event loop.
	 *
	 * \param func function to call
	 * \return handle that can be used in unregisterDeviceConnected()
	 */
	int registerDeviceConnected(const std::function<void(Device&)> &func);
	/**
	 * Register a fucntion that is called when a device is removed.
	 *
	 * The function is called asynchronously from a context event loop.
	 *
	 * \param func function to call
	 * \return handle that can be used in unregisterDeviceDisconnected()
	 */
	int registerDeviceDisconnected(const std::function<void(Device&)> &func);

	/**
	 * Unregister a device connected callback function.
	 *
	 * \param handle handle returned by registerDeviceConnected
	 */
	void unregisterDeviceConnected(int handle);
	/**
	 * Unregister a device disconnected callback function.
	 *
	 * \param handle handle returned by registerDeviceDisconnected
	 */
	void unregisterDeviceDisconnected(int handle);

	// in this case Impl must be public for the hotplug handler to be able to access it
	class Impl;
private:
	std::unique_ptr<Impl> pimpl;
};

}

#endif
