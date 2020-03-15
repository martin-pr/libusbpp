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

#include "device.h"

#include <cstring>
#include <cassert>
#include <libusb.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <vector>

#include "deviceimpl.h"

namespace Usbpp {

DeviceOpenException::DeviceOpenException(int error) noexcept : Exception(error) {

}

DeviceOpenException::~DeviceOpenException() {

}

const char* DeviceOpenException::what() const noexcept {
	return "Cannot open the device!";
}

DeviceTransferException::DeviceTransferException(int error) noexcept : Exception(error) {

}

DeviceTransferException::~DeviceTransferException() {

}

const char* DeviceTransferException::what() const noexcept {
	return "Transfer failed!";
}

Device::Impl::Impl() :
	m_device(nullptr),
	m_handle(nullptr),
	m_handleRefCount(nullptr),
	m_interfaceRefCount(nullptr) {

}

Device::Impl::Impl(libusb_device* device_) :
	m_device(device_),
	m_handle(nullptr),
	m_handleRefCount(nullptr),
	m_interfaceRefCount(nullptr) {

}

Device::Impl::Impl(const Impl& other) :
	m_device(other.m_device),
	m_handle(other.m_handle),
	m_handleRefCount(other.m_handleRefCount),
	m_interfaceMyClaimed(other.m_interfaceMyClaimed),
	m_interfaceRefCount(other.m_interfaceRefCount) {

	if (m_device) {
		libusb_ref_device(m_device);
	}
	if (m_handleRefCount) {
		++(*m_handleRefCount);
	}
}

Device::Impl::~Impl() {
	// close handle (decreases the handle and interface refcounts, too)
	close();
	// decrease the device refcount
	if (m_device) {
		libusb_unref_device(m_device);
		m_device = nullptr;
	}
}

void Device::Impl::close() {
	// release interfaces claimed by this object
	while (m_interfaceMyClaimed.begin() != m_interfaceMyClaimed.end()) {
		releaseInterface(*m_interfaceMyClaimed.begin());
	}
	// close the device
	if (m_handleRefCount) {
		--(*m_handleRefCount);
		if (*m_handleRefCount == 0) {
			libusb_close(m_handle);
			m_handle = nullptr;
			delete m_handleRefCount;
			m_handleRefCount = nullptr;
		}
	}
}

void Device::Impl::releaseInterface(int bInterfaceNumber) {
	std::unordered_set<int>::iterator it(m_interfaceMyClaimed.find(bInterfaceNumber));
	if (it != m_interfaceMyClaimed.end()) {
		m_interfaceMyClaimed.erase(it);
		std::unordered_map<int, int>::iterator refcnt(m_interfaceRefCount->find(bInterfaceNumber));
		--(refcnt->second);
		if (refcnt->second == 0) {
			libusb_release_interface(m_handle, bInterfaceNumber);
			m_interfaceRefCount->erase(refcnt);
		}
		if (m_interfaceRefCount->empty()) {
			delete m_interfaceRefCount;
			m_interfaceRefCount = nullptr;
		}
	}
}

Device::Device() : pimpl(new Impl) {

}

Device::Device(const Device& other) : pimpl(new Impl(*other.pimpl)) {

}

Device::Device(Device&& other) noexcept : pimpl(std::move(other.pimpl)) {

}

Device::Device(libusb_device* device_) : pimpl(new Impl(device_)) {

}

Device::~Device() {

}

bool Device::operator==(const Device& other) const {
	return pimpl->m_device == other.pimpl->m_device;
}

bool Device::operator!=(const Device& other) const {
	return pimpl->m_device != other.pimpl->m_device;
}

bool Device::isValid() const {
	return pimpl->m_device != nullptr;
}

Device& Device::operator=(const Device& other) {
	if (this != &other) {
		Device tmp(other);
		std::swap(pimpl, tmp.pimpl);
	}

	return *this;
}

Device& Device::operator=(Device&& other) noexcept {
	if (this != &other) {
		pimpl = std::move(other.pimpl);
	}

	return *this;
}

void Device::open(bool detachDriver) {
	if (! pimpl->m_handleRefCount) {
		int res(libusb_open(pimpl->m_device, &pimpl->m_handle));
		if (res != 0) {
			throw DeviceOpenException(res);
		}
		pimpl->m_handleRefCount = new int;
		*pimpl->m_handleRefCount = 1;
	}
	libusb_set_auto_detach_kernel_driver(pimpl->m_handle, detachDriver);
}

void Device::close() {
	pimpl->close();
}

bool Device::reset() {
	assert(pimpl->m_handle != 0);
	assert(pimpl->m_interfaceMyClaimed.empty());
	assert(pimpl->m_interfaceRefCount == 0 || pimpl->m_interfaceRefCount->empty());
	if (libusb_reset_device(pimpl->m_handle) == LIBUSB_ERROR_NOT_FOUND) {
		return false;
	}
	return true;
}

void Device::clearHalt(unsigned char endpoint) {
	libusb_clear_halt(pimpl->m_handle, endpoint);
}

libusb_device_descriptor Device::getDescriptor() {
	libusb_device_descriptor desc;
	libusb_get_device_descriptor(pimpl->m_device, &desc);
	return desc;
}

std::string* Device::getStringDescriptor(const int key) {
    /** Pre-allocated maximum length for getStringDescriptor calls. Calls with maxlen > n will throw */
    const int STRING_DESCRIPTOR_MAXLEN = 1024;
    /** Pre-allocated buffer for libusb_get_string_descriptor_ascii, using above constant */
    unsigned char data[STRING_DESCRIPTOR_MAXLEN];
    if (pimpl->m_handle == NULL) {
        return nullptr;
    }
    // pre-zero
    std::memset(data, 0, STRING_DESCRIPTOR_MAXLEN);
    libusb_get_string_descriptor_ascii(pimpl->m_handle, key, data, STRING_DESCRIPTOR_MAXLEN);
    uint16_t data_length;
    for (data_length=0; data_length<STRING_DESCRIPTOR_MAXLEN; data_length++) {
        if (data[data_length] == 0) break;
    }
    // return null when no result, don't bother allocating an empty string
    if (data_length == 0) {
        return nullptr;
    }
    return new std::string(reinterpret_cast<const char *>(data));
}

uint8_t Device::getBusNumber() const{
    if (pimpl->m_device == NULL) {
        return 0;
    }

    return libusb_get_bus_number(pimpl->m_device);
}

uint8_t Device::getDeviceAddress() const {
    return libusb_get_device_address(pimpl->m_device);
}

std::vector<uint8_t> *Device::getPortNumbers() {
    std::vector<uint8_t> *ports_vec = new std::vector<uint8_t>();

    if (pimpl->m_device != NULL) {
        uint8_t ports[Usbpp::Device::MAX_PORT_NUMBERS];
        int count = libusb_get_port_numbers(pimpl->m_device, ports, Usbpp::Device::MAX_PORT_NUMBERS);
        if (count > 0) {
            ports_vec->reserve(count);
            for (int i = 0; i < count; i++) {
                ports_vec->push_back(ports[i]);
            }
        }
    }
    return ports_vec;
}

int Device::getConfiguration() {
	int config;
	int res = libusb_get_configuration(pimpl->m_handle, &config);
	if (res < 0) {
		throw DeviceTransferException(res);
	}
	return config;
}

void Device::setConfiguration(int bConfigurationValue) {
	int res = libusb_set_configuration(pimpl->m_handle, bConfigurationValue);
	if (res < 0) {
		throw DeviceTransferException(res);
	}
}

void Device::claimInterface(int bInterfaceNumber) {
	pimpl->m_interfaceMyClaimed.emplace(bInterfaceNumber);
	if (! pimpl->m_interfaceRefCount) {
		pimpl->m_interfaceRefCount = new std::unordered_map<int, int>();
	}
	std::unordered_map<int, int>::iterator refcnt(pimpl->m_interfaceRefCount->find(bInterfaceNumber));
	if (refcnt == pimpl->m_interfaceRefCount->end()) {
		pimpl->m_interfaceRefCount->insert(std::make_pair(bInterfaceNumber, 1));
	}
	else {
		++(refcnt->second);
	}
	libusb_claim_interface(pimpl->m_handle, bInterfaceNumber);
}

void Device::releaseInterface(int bInterfaceNumber) {
	pimpl->releaseInterface(bInterfaceNumber);
}

int Device::controlTransferIn(uint8_t bmRequestType,
                              uint8_t bRequest,
                              uint16_t wValue,
                              uint16_t wIndex,
                              ByteBuffer& data,
                              unsigned int timeout) const {
	assert(bmRequestType & LIBUSB_ENDPOINT_IN);
	int res = libusb_control_transfer(pimpl->m_handle, bmRequestType, bRequest, wValue, wIndex, data.data(), data.size(), timeout);
	if (res < 0) {
		throw DeviceTransferException(res);
	}
	return res;
}

int Device::bulkTransferIn(unsigned char endpoint,
                           ByteBuffer& data,
                           unsigned int timeout) const {
	assert(endpoint & LIBUSB_ENDPOINT_IN);
	int transferred(0);
	int res = libusb_bulk_transfer(pimpl->m_handle, endpoint, data.data(), data.size(), &transferred, timeout);
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

int Device::interruptTransferIn(unsigned char endpoint,
                                ByteBuffer& data,
                                unsigned int timeout) const {
	assert(endpoint & LIBUSB_ENDPOINT_IN);
	int transferred(0);
	int res = libusb_interrupt_transfer(pimpl->m_handle, endpoint, data.data(), data.size(), &transferred, timeout);
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

int Device::controlTransferOut(uint8_t bmRequestType,
                               uint8_t bRequest,
                               uint16_t wValue,
                               uint16_t wIndex,
                               const ByteBuffer& data,
                               unsigned int timeout) const {
	assert((bmRequestType & LIBUSB_ENDPOINT_IN) == 0);
	int res = libusb_control_transfer(pimpl->m_handle, bmRequestType, bRequest, wValue, wIndex,
	                                  const_cast<unsigned char*>(data.data()), data.size(), timeout);
	if (res < 0) {
		throw DeviceTransferException(res);
	}
	return res;
}

int Device::bulkTransferOut(unsigned char endpoint,
                            const ByteBuffer& data,
                            unsigned int timeout) const {
	assert((endpoint & LIBUSB_ENDPOINT_IN) == 0);
	int transferred(0);
	int res = libusb_bulk_transfer(pimpl->m_handle, endpoint,
	                               const_cast<unsigned char*>(data.data()), data.size(), &transferred, timeout);
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

int Device::interruptTransferOut(unsigned char endpoint,
                                 const ByteBuffer& data,
                                 unsigned int timeout) const {
	assert((endpoint & LIBUSB_ENDPOINT_IN) == 0);
	int transferred(0);
	int res = libusb_interrupt_transfer(pimpl->m_handle, endpoint,
	                                    const_cast<unsigned char*>(data.data()), data.size(), &transferred, timeout);
	if (res != 0) {
		throw DeviceTransferException(res);
	}
	return transferred;
}

}
