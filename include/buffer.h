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
	/**
	 * Default constructor.
	 *
	 * Constructs a buffer with no storage and zero size.
	 */
	ByteBuffer();
	/**
	 * Construct a buffer with a specified size.
	 *
	 * The data are not initialized.
	 *
	 * \param size Size of the buffer.
	 */
	explicit ByteBuffer(std::size_t size);
	/**
	 * Constructs a buffer from existing byte buffer.
	 *
	 * The data are copied from the array pointed by \a data to a new buffer.
	 *
	 * \param data Buffer containing data.
	 * \param size Size of the data buffer in bytes.
	 */
	ByteBuffer(const std::uint8_t* data, std::size_t size);
	/**
	 * Copy constructor.
	 */
	ByteBuffer(const ByteBuffer& other);
	/**
	 * Move constructor.
	 */
	ByteBuffer(ByteBuffer&& other) noexcept;
	/**
	 * A destructor.
	 */
	~ByteBuffer();

	/**
	 * Assignment operator.
	 */
	ByteBuffer& operator=(const ByteBuffer& other);
	/**
	 * Move assignment operator.
	 */
	ByteBuffer& operator=(ByteBuffer&& other) noexcept;

	/**
	 * Byte access operator.
	 *
	 * Accesses byte at a specified position.
	 *
	 * \param i Byte to access.
	 * \return Reference to \a i-th byte.
	 */
	std::uint8_t& operator[](std::size_t i);
	/**
	 * \copydoc std::uint8_t& operator[](std::size_t i)
	 *
	 * Const overload.
	 */
	const std::uint8_t& operator[](std::size_t i) const;

	/**
	 * Append another buffer.
	 *
	 * The data of \a other buffer are appended at the end of the current buffer
	 * and the current buffer is resized accordingly.
	 *
	 * \param other Buffer to append.
	 * \return The resulting buffer.
	 */
	ByteBuffer& append(const ByteBuffer& other);

	/**
	 * Resize the buffer.
	 *
	 * \param size New buffer size.
	 */
	void resize(std::size_t size);

	/**
	 * Get the current buffer size.
	 */
	std::size_t size() const ;

	/**
	 * Get pointer to data held by the buffer.
	 */
	std::uint8_t* data();

	/**
	 * \copydoc std::uint8_t* data()
	 *
	 * Const overload.
	 */
	const std::uint8_t* data() const;

private:
	std::uint8_t* m_data;
	std::size_t m_size;
};

}

#endif
