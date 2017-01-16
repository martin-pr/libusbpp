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

#include <string>

namespace Usbpp {

/**
 * An exception thrown when a libusb error occurs.
 */
class Exception {
public:
	/**
	 * Construct an exception caused by a libusb error.
	 *
	 * \param error Libusb error (see libusb_error enum in libusb).
	 */
	explicit Exception(int error) noexcept;
	/**
	 * A destructor.
	 */
	virtual ~Exception();

	/**
	 * Return the libusb error that caused the exception.
	 *
	 * \return The numeric value of the error.
	 */
	int getError() const;

	/**
	 * Return brief description of the exception.
	 */
	virtual const char* what() const noexcept = 0;

	/**
	 * Return textual representation of the exception for showing to user.
	 *
	 * The default implementation returns string consisting of the description
	 * returned by \a what() and the string representation of the libusb error.
	 */
	virtual std::string getDescription() const;

private:
	const int m_error;
};

}

#endif
