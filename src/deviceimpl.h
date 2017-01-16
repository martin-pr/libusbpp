/*
 * This file is part of Usbpp, a C++ wrapper around libusb
 * Copyright (C) 2016  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
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

#include <unordered_map>
#include <unordered_set>

#include <libusb.h>

namespace Usbpp {

class Device::Impl {
public:
	Impl();
	explicit Impl(libusb_device* device_);
	Impl(const Impl& other);
	~Impl();

	void close();
	void releaseInterface(int bInterfaceNumber);

	libusb_device* m_device;
	libusb_device_handle* m_handle;
	int* m_handleRefCount;
	// a set of interfaces claimed by the device
	std::unordered_set<int> m_interfaceMyClaimed;
	// a shared map storing the reference counts for all interfaces
	std::unordered_map<int, int>* m_interfaceRefCount;
};

}
