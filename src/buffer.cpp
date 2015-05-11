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

namespace Usbpp {

ByteBuffer::ByteBuffer() : mdata(nullptr), msize(0) {

}

ByteBuffer::ByteBuffer(std::size_t msize_) {
	mdata = static_cast<std::uint8_t*>(malloc(msize_ * sizeof(std::uint8_t)));
	msize = msize_;
}

ByteBuffer::ByteBuffer(const std::uint8_t* data_, std::size_t msize_) {
	mdata = static_cast<std::uint8_t*>(malloc(msize_ * sizeof(std::uint8_t)));
	std::memcpy(mdata, data_, msize_ * sizeof(std::uint8_t));
	msize = msize_;
}

ByteBuffer::ByteBuffer(const ByteBuffer &other) {
	mdata = static_cast<std::uint8_t*>(malloc(other.msize * sizeof(std::uint8_t)));
	std::memcpy(mdata, other.mdata, other.msize * sizeof(std::uint8_t));
	msize = other.msize;
}

ByteBuffer::ByteBuffer(ByteBuffer &&other) noexcept {
	mdata = other.mdata;
	other.mdata = nullptr;
	msize = other.msize;
	other.msize = 0;
}

ByteBuffer::~ByteBuffer() {
	if (mdata) {
		free(mdata);
		mdata = nullptr;
		msize = 0;
	}
}

ByteBuffer &ByteBuffer::operator=(const ByteBuffer &other) {
	if (this == &other) {
		return *this;
	}
	if(msize == other.msize) {
		std::memcpy(mdata, other.mdata, msize);
		return *this;
	}
	ByteBuffer tmp(other);
	std::swap(mdata, tmp.mdata);
	std::swap(msize, tmp.msize);
	return *this;
}

ByteBuffer &ByteBuffer::operator=(ByteBuffer &&other) noexcept {
	if (this == &other) {
		return *this;
	}
	std::swap(mdata, other.mdata);
	std::swap(msize, other.msize);
	return *this;
}

std::uint8_t &ByteBuffer::operator[](std::size_t i) {
	return mdata[i];
}

const std::uint8_t &ByteBuffer::operator[](std::size_t i) const {
	return mdata[i];
}

ByteBuffer &ByteBuffer::append(const ByteBuffer &other) {
	std::uint8_t *tmp(static_cast<std::uint8_t*>(realloc(mdata, msize + other.msize)));
	if (tmp == nullptr) {
		// TODO: throw
	}
	// copy the contents
	memcpy(tmp + msize, other.mdata, other.msize);
	// update the current buffer
	mdata = tmp;
	msize += other.msize;

	return *this;
}

void ByteBuffer::resize(std::size_t size) {
	if(size == msize) {
		return;
	}
	std::uint8_t *tmp(static_cast<std::uint8_t*>(realloc(mdata, size)));
	if (tmp == nullptr) {
		// TODO: throw
	}
	mdata = tmp;
	msize = size;
}

std::size_t ByteBuffer::size() const {
	return msize;
}

std::uint8_t* ByteBuffer::data() {
	return mdata;
}

const std::uint8_t* ByteBuffer::data() const {
	return mdata;
}

}
