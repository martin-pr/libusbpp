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
#ifndef LIBUSBPP_EXCEPTION_H_
#define LIBUSBPP_EXCEPTION_H_

namespace Usbpp {

/**
 * An exception thrown when a libusb error occurs.
 */
class Exception {
public:
	explicit Exception(int error) noexcept;
	virtual ~Exception();

	int getError() const;

	virtual const char* what() const noexcept = 0;
private:
	const int error;
};


}

#endif
