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

#include "buffer.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <new>

namespace Usbpp {

ByteBuffer::ByteBuffer()
	: m_data(nullptr), m_size(0) {

}

ByteBuffer::ByteBuffer(std::size_t size)
	: m_size(size) {
	m_data = static_cast<std::uint8_t*>(malloc(size * sizeof(std::uint8_t)));
	if (m_data == nullptr) {
		throw std::bad_alloc();
	}
}

ByteBuffer::ByteBuffer(const std::uint8_t* data_, std::size_t size)
	: m_size(size) {
	m_data = static_cast<std::uint8_t*>(malloc(size * sizeof(std::uint8_t)));
	if (m_data == nullptr) {
		throw std::bad_alloc();
	}
	std::memcpy(m_data, data_, size * sizeof(std::uint8_t));
}

ByteBuffer::ByteBuffer(const ByteBuffer& other)
	:m_size(other.m_size) {
	m_data = static_cast<std::uint8_t*>(malloc(other.m_size * sizeof(std::uint8_t)));
	if (m_data == nullptr) {
		throw std::bad_alloc();
	}
	std::memcpy(m_data, other.m_data, other.m_size * sizeof(std::uint8_t));
}

ByteBuffer::ByteBuffer(ByteBuffer&& other) noexcept
	: m_data(other.m_data), m_size(other.m_size) {
	other.m_data = nullptr;
	other.m_size = 0;
}

ByteBuffer::~ByteBuffer() {
	if (m_data) {
		free(m_data);
		m_data = nullptr;
		m_size = 0;
	}
}

ByteBuffer& ByteBuffer::operator=(const ByteBuffer& other) {
	if (this != &other) {
		if (m_size == other.m_size) {
			std::memcpy(m_data, other.m_data, m_size);
			return *this;
		}
		ByteBuffer tmp(other);
		std::swap(m_data, tmp.m_data);
		std::swap(m_size, tmp.m_size);
	}
	return *this;
}

ByteBuffer& ByteBuffer::operator=(ByteBuffer&& other) noexcept {
	if (this != &other) {
		if (m_data) {
			free(m_data);
		}
		m_data = other.m_data;
		m_size = other.m_size;
		other.m_data = nullptr;
		other.m_size = 0;
	}
	return *this;
}

std::uint8_t& ByteBuffer::operator[](std::size_t i) {
	return m_data[i];
}

const std::uint8_t& ByteBuffer::operator[](std::size_t i) const {
	return m_data[i];
}

ByteBuffer& ByteBuffer::append(const ByteBuffer& other) {
	std::uint8_t* tmp(static_cast<std::uint8_t*>(realloc(m_data, m_size + other.m_size)));
	if (tmp == nullptr) {
		throw std::bad_alloc();
	}
	// copy the contents
	memcpy(tmp + m_size, other.m_data, other.m_size);
	// update the current buffer
	m_data = tmp;
	m_size += other.m_size;

	return *this;
}

void ByteBuffer::resize(std::size_t size) {
	if (size == m_size) {
		return;
	}
	std::uint8_t* tmp(static_cast<std::uint8_t*>(realloc(m_data, size)));
	if (tmp == nullptr) {
		throw std::bad_alloc();
	}
	m_data = tmp;
	m_size = size;
}

std::size_t ByteBuffer::size() const {
	return m_size;
}

std::uint8_t* ByteBuffer::data() {
	return m_data;
}

const std::uint8_t* ByteBuffer::data() const {
	return m_data;
}

}
