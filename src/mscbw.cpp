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

#include "mscbw.h"

#include <cassert>
#include <cstring>

namespace {
constexpr std::size_t CBW_LEN = 31;
}

namespace Usbpp {
namespace MassStorage {

CommandBlockWrapper::CommandBlockWrapper() : m_data(CBW_LEN) {
	std::memset(&m_data[0], 0, CBW_LEN);
}

CommandBlockWrapper::CommandBlockWrapper(const ByteBuffer& buffer) {
	assert(buffer.size() <= CBW_LEN);

	m_data = buffer;
	m_data.resize(CBW_LEN);
	// fill the rest with zeroes
	if (buffer.size() != CBW_LEN) {
		std::memset(&m_data[buffer.size()], 0, CBW_LEN - buffer.size());
	}
}

CommandBlockWrapper::CommandBlockWrapper(uint32_t dCBWDataTransferLength,
                                         uint8_t bmCBWFlags,
                                         uint8_t bCBWLUN,
                                         std::vector<uint8_t> CBWCB) :
	m_data(CBW_LEN) {
	// only CBWCB needs to be zeroed as the rest will be written into
	std::memset(&m_data[15], 0, CBW_LEN - 15);

	// dCBWSignature
	m_data[0] = 'U';
	m_data[1] = 'S';
	m_data[2] = 'B';
	m_data[3] = 'C';
	// dCBWTag
	uint32_t tag(generateTag());
	m_data[4] = tag & 0xFF;
	m_data[5] = (tag >> 8) & 0xFF;
	m_data[6] = (tag >> 16) & 0xFF;
	m_data[7] = (tag >> 24) & 0xFF;
	// dCBWDataTransferLength
	m_data[8] = dCBWDataTransferLength & 0xFF;
	m_data[9] = (dCBWDataTransferLength >> 8) & 0xFF;
	m_data[10] = (dCBWDataTransferLength >> 16) & 0xFF;
	m_data[11] = (dCBWDataTransferLength >> 24) & 0xFF;
	// bmCBWFlags
	assert((bmCBWFlags & 0x3F) ==  0); // reserved bits
	assert((bmCBWFlags & 0x40) ==  0); // obsolete bits
	m_data[12] = bmCBWFlags;
	// bCBWLUN
	assert((bCBWLUN & 0xF) ==  bCBWLUN);
	m_data[13] = bCBWLUN;
	// bCBWCBLength
	assert((CBWCB.size() & 0x1F) ==  CBWCB.size());
	m_data[14] = CBWCB.size() & 0x1F;
	// CBWCB
	std::memcpy(&m_data[15], &CBWCB[0], CBWCB.size());
}

CommandBlockWrapper::~CommandBlockWrapper() {

}

CommandBlockWrapper::CommandBlockWrapper(const CommandBlockWrapper& other) : m_data(other.m_data) {

}

CommandBlockWrapper::CommandBlockWrapper(CommandBlockWrapper&& other) noexcept
	: m_data(std::move(other.m_data)) {

}

CommandBlockWrapper& CommandBlockWrapper::operator=(const CommandBlockWrapper& other) {
	if (this != &other) {
		m_data = other.m_data;
	}

	return *this;
}

CommandBlockWrapper& CommandBlockWrapper::operator=(CommandBlockWrapper&& other) noexcept {
	if (this != &other) {
		m_data = std::move(other.m_data);
	}

	return *this;
}

uint32_t CommandBlockWrapper::getTag() const {
	uint32_t tag;
	tag = m_data[7];
	tag = (tag << 8) | m_data[6];
	tag = (tag << 8) | m_data[5];
	tag = (tag << 8) | m_data[4];
	return tag;
}

uint32_t CommandBlockWrapper::getTransferLength() const {
	uint32_t len;
	len = m_data[11];
	len = (len << 8) | m_data[10];
	len = (len << 8) | m_data[9];
	len = (len << 8) | m_data[8];
	return len;
}

CommandBlockWrapper::Flags CommandBlockWrapper::getFlags() const {
	if ((m_data[12] & static_cast<uint8_t>(Flags::INVALID)) != 0) {
		return Flags::INVALID;
	}
	return static_cast<Flags>(m_data[12]);
}

uint8_t CommandBlockWrapper::getLun() const {
	return m_data[13];
}

uint8_t CommandBlockWrapper::getCommandBlockLength() const {
	return m_data[14];
}

std::vector< uint8_t > CommandBlockWrapper::getCommandBlock() const {
	return std::vector<uint8_t>(&m_data[15], &m_data[15] + getCommandBlockLength());
}

const ByteBuffer& CommandBlockWrapper::getBuffer() const {
	return m_data;
}

uint32_t CommandBlockWrapper::generateTag() const {
	static uint32_t tag;
	return tag++;
}

std::ostream& operator<<(std::ostream& os, const CommandBlockWrapper::Flags& flags) {
	switch (flags) {
		case Usbpp::MassStorage::CommandBlockWrapper::Flags::DATA_IN:
			return os << "DATA_IN (0x00)";
		case Usbpp::MassStorage::CommandBlockWrapper::Flags::DATA_OUT:
			return os << "DATA_OUT (0x80)";
		case Usbpp::MassStorage::CommandBlockWrapper::Flags::INVALID:
			return os << "INVALID (0x7F)";
	}

	return os;
}

}
}
