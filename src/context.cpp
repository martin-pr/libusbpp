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

#include "context.h"

#include <cassert>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <unordered_map>

#include <libusb.h>

namespace Usbpp {

ContextInitException::ContextInitException(int error) noexcept : Exception(error) {

}

ContextInitException::~ContextInitException() {

}

const char* ContextInitException::what() const noexcept {
	return "Cannot initialize context";
}

ContextEnumerateException::ContextEnumerateException(int error) noexcept : Exception(error) {

}

ContextEnumerateException::~ContextEnumerateException() {

}

const char* ContextEnumerateException::what() const noexcept {
	return "Cannot initialize context";
}

ContextRegisterCBException::ContextRegisterCBException(int error) noexcept : Exception(error) {

}

ContextRegisterCBException::~ContextRegisterCBException() {

}

const char* ContextRegisterCBException::what() const noexcept {
	return "Cannot register callback";
}

class Context::Impl {
public:
	Impl();
	Impl(const Impl &other);
	~Impl();
	/**
	 * Event loop implementation
	 */
	void eventLoop();
	/**
	 * Enter the event loop checking for callbacks
	 */
	void startEventLoop();
	/**
	 * Exit the event loop checking for callbacks
	 */
	void stopEventLoop();
	/**
	 * Handle a single callback event
	 */
	void handleEvent(libusb_device *device, libusb_hotplug_event event);

	using DeviceMap = std::unordered_map<libusb_device *, Device>;
	using CallbackMap = std::unordered_map<int, std::function<void(Device&)>>;

	static int handleGenerator;
	int *refcount;
	libusb_context *ctx;
	// hotplug callback handling
	bool hotplugEnabled;
	std::thread hotplugThread;
	libusb_hotplug_callback_handle hotplugHandle;
	DeviceMap devices;
	CallbackMap funcConnected;
	CallbackMap funcDisconnected;
};

int eventHandler(libusb_context *, libusb_device *device, libusb_hotplug_event event, void *user_data) {
	Context::Impl* contextimpl = static_cast<Context::Impl*>(user_data);
	contextimpl->handleEvent(device, event);
	return LIBUSB_SUCCESS;
}

int Context::Impl::handleGenerator = 0;

Context::Impl::Impl() {
	refcount = new int;
	*refcount = 1;
	int res = libusb_init(&ctx);
	if (res != 0) {
		delete refcount;
		throw ContextInitException(res);
	}

	hotplugEnabled = false;
}

Context::Impl::Impl(const Usbpp::Context::Impl& other): refcount(other.refcount), ctx(other.ctx) {
	++(*refcount);
}

Context::Impl::~Impl() {
	if (refcount) {
		--(*refcount);
		if (*refcount == 0) {
			libusb_exit(ctx);
			ctx = nullptr;
			delete refcount;
			refcount = nullptr;
		}
	}
}

void Context::Impl::eventLoop() {
	while (hotplugEnabled) {
		libusb_handle_events_completed(NULL, NULL);
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

void Context::Impl::startEventLoop() {
	if (hotplugEnabled) {
		return;
	}

	hotplugEnabled = true;
	int res = libusb_hotplug_register_callback(ctx,
	                                           static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
	                                           LIBUSB_HOTPLUG_NO_FLAGS,
	                                           LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
	                                           LIBUSB_HOTPLUG_MATCH_ANY, eventHandler, this,
	                                           &hotplugHandle);
	if (res != LIBUSB_SUCCESS) {
		hotplugEnabled = false;
		throw ContextRegisterCBException(res);
	}

	hotplugThread = std::thread(&Impl::eventLoop, this);
}

void Context::Impl::stopEventLoop() {
	if (!hotplugEnabled) {
		return;
	}

	hotplugEnabled = false;
	hotplugThread.join();
	libusb_hotplug_deregister_callback(ctx, hotplugHandle);
}

void Context::Impl::handleEvent(libusb_device *usbdevice, libusb_hotplug_event event) {
	switch (event) {
		case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED: {
			// insert device to the internal map
			if (devices.find(usbdevice) != devices.end()) {
				devices.insert(std::make_pair(usbdevice, Device(usbdevice)));
			}
			// find device
			Device& device = devices.at(usbdevice);
			// execute the callbacks
			for (auto &func : funcConnected) {
				func.second(device);
			}
			break;
		}
		case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT: {
			// get device for which to generate callback
			DeviceMap::iterator it(devices.find(usbdevice));
			Device device = (it != devices.end() ? it->second : Device(usbdevice));
			// execute the callbacks
			for (auto &func : funcDisconnected) {
				func.second(device);
			}
			// erase the device from internal map
			devices.erase(usbdevice);
			break;
		}
		default:
			// do nothing
			break;
	}
}

Context::Context() : pimpl(new Impl) {

}

Context::Context(const Context& other) : pimpl(new Impl(*(other.pimpl))) {

}

Context::Context(Context&& other) noexcept: pimpl(std::move(other.pimpl)) {

}

Context::~Context() {

}

Context& Context::operator=(const Context& other) {
	if (this != &other) {
		// both share the same refcount pointer => they point to the same context
		if (pimpl->refcount ==other.pimpl->refcount) {
			assert(pimpl->ctx == other.pimpl->ctx);
			return *this;
		}

		std::unique_ptr<Impl> tmp(new Impl(*(other.pimpl)));
		std::swap(tmp, pimpl);
	}

	return *this;
}

Context& Context::operator=(Context &&other) noexcept {
	if (this != &other) {
		pimpl = std::move(other.pimpl);
	}

	return *this;
}

std::vector<Device> Context::getDevices() {
	libusb_device **devices;
	int count = libusb_get_device_list(pimpl->ctx, &devices);
	if (count < 0) {
		throw ContextEnumerateException(count);
	}

	std::vector<Device> devicesRes;
	devicesRes.reserve(count);
	for (int i(0); i < count; ++i) {
		Device device(devices[i]);
		devicesRes.push_back(device);
		pimpl->devices.insert(std::make_pair(devices[i], device));
	}

	libusb_free_device_list(devices, 0);

	return devicesRes;
}

int Context::registerDeviceConnected(const std::function<void(Device&)> &func) {
	int handle = pimpl->handleGenerator++;
	pimpl->funcConnected.insert(std::make_pair(handle, func));
	pimpl->startEventLoop();
	return handle;
}

int Context::registerDeviceDisconnected(const std::function<void(Device&)> &func) {
	int handle = pimpl->handleGenerator++;
	pimpl->funcDisconnected.insert(std::make_pair(handle, func));
	pimpl->startEventLoop();
	return handle;
}

void Context::unregisterDeviceConnected(int handle) {
	pimpl->funcConnected.erase(handle);
	if (pimpl->funcConnected.empty() && pimpl->funcDisconnected.empty()) {
		pimpl->stopEventLoop();
	}
}

void Context::unregisterDeviceDisconnected(int handle) {
	pimpl->funcDisconnected.erase(handle);
	if (pimpl->funcConnected.empty() && pimpl->funcDisconnected.empty()) {
		pimpl->stopEventLoop();
	}
}

}
