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
	return "Cannot initialize context!";
}

ContextEnumerateException::ContextEnumerateException(int error) noexcept : Exception(error) {

}

ContextEnumerateException::~ContextEnumerateException() {

}

const char* ContextEnumerateException::what() const noexcept {
	return "Cannot initialize context!";
}

ContextRegisterCBException::ContextRegisterCBException(int error) noexcept : Exception(error) {

}

ContextRegisterCBException::~ContextRegisterCBException() {

}

const char* ContextRegisterCBException::what() const noexcept {
	return "Cannot register callback!";
}

class Context::Impl {
public:
	Impl();
	Impl(const Impl& other);
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
	void handleEvent(libusb_device* device, libusb_hotplug_event event);

	using DeviceMap = std::unordered_map<libusb_device*, Device>;
	using CallbackMap = std::unordered_map<int, std::function<void(Device&)>>;

	static int m_handleGenerator;
	int* m_refcount;
	libusb_context* m_ctx;
	// hotplug callback handling
	bool m_hotplugEnabled;
	std::thread m_hotplugThread;
	libusb_hotplug_callback_handle m_hotplugHandle;
	DeviceMap m_devices;
	CallbackMap m_funcConnected;
	CallbackMap m_funcDisconnected;
};

}

namespace {
/**
 * Free function serving as libusb even handler
 */
int eventHandler(libusb_context*, libusb_device* device, libusb_hotplug_event event, void* user_data) {
	Usbpp::Context::Impl* contextimpl = static_cast<Usbpp::Context::Impl*>(user_data);
	contextimpl->handleEvent(device, event);
	return LIBUSB_SUCCESS;
}
}

namespace Usbpp {

int Context::Impl::m_handleGenerator = 0;

Context::Impl::Impl() {
	m_refcount = new int;
	*m_refcount = 1;
	int res = libusb_init(&m_ctx);
	if (res != 0) {
		delete m_refcount;
		throw ContextInitException(res);
	}

	m_hotplugEnabled = false;
}

Context::Impl::Impl(const Usbpp::Context::Impl& other): m_refcount(other.m_refcount), m_ctx(other.m_ctx) {
	++(*m_refcount);
}

Context::Impl::~Impl() {
	if (m_refcount) {
		--(*m_refcount);
		if (*m_refcount == 0) {
			libusb_exit(m_ctx);
			m_ctx = nullptr;
			delete m_refcount;
			m_refcount = nullptr;
		}
	}
}

void Context::Impl::eventLoop() {
	while (m_hotplugEnabled) {
		libusb_handle_events_completed(NULL, NULL);
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

void Context::Impl::startEventLoop() {
	if (m_hotplugEnabled) {
		return;
	}

	m_hotplugEnabled = true;
	int res = libusb_hotplug_register_callback(m_ctx,
	                                           static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
	                                           LIBUSB_HOTPLUG_NO_FLAGS,
	                                           LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
	                                           LIBUSB_HOTPLUG_MATCH_ANY, eventHandler, this,
	                                           &m_hotplugHandle);
	if (res != LIBUSB_SUCCESS) {
		m_hotplugEnabled = false;
		throw ContextRegisterCBException(res);
	}

	m_hotplugThread = std::thread(&Impl::eventLoop, this);
}

void Context::Impl::stopEventLoop() {
	if (!m_hotplugEnabled) {
		return;
	}

	m_hotplugEnabled = false;
	m_hotplugThread.join();
	libusb_hotplug_deregister_callback(m_ctx, m_hotplugHandle);
}

void Context::Impl::handleEvent(libusb_device* usbdevice, libusb_hotplug_event event) {
	switch (event) {
		case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED: {
			// insert device to the internal map
			if (m_devices.find(usbdevice) != m_devices.end()) {
				m_devices.insert(std::make_pair(usbdevice, Device(usbdevice)));
			}
			// find device
			Device& device = m_devices.at(usbdevice);
			// execute the callbacks
			for (auto& func : m_funcConnected) {
				func.second(device);
			}
			break;
		}
		case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT: {
			// get device for which to generate callback
			DeviceMap::iterator it(m_devices.find(usbdevice));
			Device device = (it != m_devices.end() ? it->second : Device(usbdevice));
			// execute the callbacks
			for (auto& func : m_funcDisconnected) {
				func.second(device);
			}
			// erase the device from internal map
			m_devices.erase(usbdevice);
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
		if (pimpl->m_refcount ==other.pimpl->m_refcount) {
			assert(pimpl->m_ctx == other.pimpl->m_ctx);
			return *this;
		}

		std::unique_ptr<Impl> tmp(new Impl(*(other.pimpl)));
		std::swap(tmp, pimpl);
	}

	return *this;
}

Context& Context::operator=(Context&& other) noexcept {
	if (this != &other) {
		pimpl = std::move(other.pimpl);
	}

	return *this;
}

template <typename ... Ts>
void Context::setOption(libusb_option option, Ts ... args) {
    libusb_set_option( pimpl->m_ctx, option, args...);
}

std::vector<Device> Context::getDevices() {
	libusb_device** devices;
	int count = libusb_get_device_list(pimpl->m_ctx, &devices);
	if (count < 0) {
		throw ContextEnumerateException(count);
	}

	std::vector<Device> devicesRes;
	devicesRes.reserve(count);
	for (int i(0); i < count; ++i) {
		Device device(devices[i]);
		devicesRes.push_back(device);
		pimpl->m_devices.insert(std::make_pair(devices[i], device));
	}

	libusb_free_device_list(devices, 0);

	return devicesRes;
}

int Context::registerDeviceConnected(const std::function<void(Device&)>& func) {
	int handle = pimpl->m_handleGenerator++;
	pimpl->m_funcConnected.insert(std::make_pair(handle, func));
	pimpl->startEventLoop();
	return handle;
}

int Context::registerDeviceDisconnected(const std::function<void(Device&)>& func) {
	int handle = pimpl->m_handleGenerator++;
	pimpl->m_funcDisconnected.insert(std::make_pair(handle, func));
	pimpl->startEventLoop();
	return handle;
}

void Context::unregisterDeviceConnected(int handle) {
	pimpl->m_funcConnected.erase(handle);
	if (pimpl->m_funcConnected.empty() && pimpl->m_funcDisconnected.empty()) {
		pimpl->stopEventLoop();
	}
}

void Context::unregisterDeviceDisconnected(int handle) {
	pimpl->m_funcDisconnected.erase(handle);
	if (pimpl->m_funcConnected.empty() && pimpl->m_funcDisconnected.empty()) {
		pimpl->stopEventLoop();
	}
}

}
