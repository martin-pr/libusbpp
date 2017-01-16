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

#include "stddevicehash.h"

#include "device.h"
#include "deviceimpl.h"

namespace std {

size_t hash<::Usbpp::Device>::operator()(const ::Usbpp::Device& device) const {
	return std::hash<libusb_device*>()(device.pimpl->m_device);
}

}
