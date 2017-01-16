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

#include "exception.h"

#include <libusb.h>

namespace Usbpp {

Exception::Exception(int error) noexcept
	: m_error(error) {

}

Exception::~Exception() {

}

int Exception::getError() const {
	return m_error;
}

std::string Exception::getDescription() const {
	return std::string(what()) + " Caused by libusb error: " + libusb_strerror(static_cast<libusb_error>(m_error));
}

}
