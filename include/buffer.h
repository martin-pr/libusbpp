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

#ifndef LIBUSBPP_BUFFER_H_
#define LIBUSBPP_BUFFER_H_

#include <cstddef>
#include <cstdint>

namespace Usbpp {

/**
 * A static-sized buffer.
 */
class ByteBuffer {
public:
	ByteBuffer();
	ByteBuffer(std::size_t msize_);
	ByteBuffer(const std::uint8_t* data_, std::size_t msize_);
	ByteBuffer(const ByteBuffer &other);
	ByteBuffer(ByteBuffer &&other) noexcept;
	~ByteBuffer();

	ByteBuffer &operator=(const ByteBuffer &other);
	ByteBuffer &operator=(ByteBuffer &&other) noexcept;

	std::uint8_t &operator[](std::size_t i);
	const std::uint8_t &operator[](std::size_t i) const;
	
	ByteBuffer &append(const ByteBuffer &other);

	void resize(std::size_t size);

	std::size_t size() const ;

	std::uint8_t* data();
	const std::uint8_t* data() const;

private:
	std::uint8_t* mdata;
	std::size_t msize;
};

}

#endif
