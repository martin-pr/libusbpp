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

#include "mscsw.h"

#include "buffer.h"

#include <cassert>
#include <cstring>

namespace {
constexpr std::size_t CSW_LEN = 13;
}

namespace Usbpp {
namespace MassStorage {

CommandStatusWrapper::CommandStatusWrapper() : m_data(CSW_LEN) {
	std::memset(&m_data[0], 0, CSW_LEN);
}

CommandStatusWrapper::CommandStatusWrapper(const ByteBuffer& buffer) {
	assert(buffer.size() == CSW_LEN);

	m_data = buffer;
}

CommandStatusWrapper::CommandStatusWrapper(uint32_t dCSWTag, uint32_t dCSWDataResidue, uint8_t bCSWStatus) : m_data(CSW_LEN) {
	std::memset(&m_data[0], 0, CSW_LEN);

	// dCSWSignature
	m_data[0] = 'U';
	m_data[1] = 'S';
	m_data[2] = 'B';
	m_data[3] = 'S';
	// dCSWTag
	m_data[4] = (dCSWTag >> 24) & 0xFF;
	m_data[5] = (dCSWTag >> 16) & 0xFF;
	m_data[6] = (dCSWTag >> 8) & 0xFF;
	m_data[7] = dCSWTag & 0xFF;
	// dCSWDataResidue
	m_data[8] = (dCSWDataResidue >> 24) & 0xFF;
	m_data[9] = (dCSWDataResidue >> 16) & 0xFF;
	m_data[10] = (dCSWDataResidue >> 8) & 0xFF;
	m_data[11] = dCSWDataResidue & 0xFF;
	// bCSWStatus
	assert(bCSWStatus != 0x03 && bCSWStatus != 0x04); // obsolete
	assert(bCSWStatus < 0x05); // reserved
	m_data[12] = bCSWStatus;
}

CommandStatusWrapper::~CommandStatusWrapper() {
}

CommandStatusWrapper::CommandStatusWrapper(const CommandStatusWrapper& other) : m_data(other.m_data) {

}

CommandStatusWrapper::CommandStatusWrapper(CommandStatusWrapper&& other) noexcept
	: m_data(std::move(other.m_data)) {

}

CommandStatusWrapper& CommandStatusWrapper::operator=(const CommandStatusWrapper& other) {
	if (this != &other) {
		m_data = other.m_data;
	}

	return *this;
}

CommandStatusWrapper& CommandStatusWrapper::operator=(CommandStatusWrapper&& other) noexcept {
	if (this != &other) {
		m_data = std::move(other.m_data);
	}

	return *this;
}

uint32_t CommandStatusWrapper::getTag() const {
	uint32_t tag;
	tag = m_data[4];
	tag = (tag << 8) | m_data[5];
	tag = (tag << 8) | m_data[6];
	tag = (tag << 8) | m_data[7];
	return tag;
}

uint32_t CommandStatusWrapper::getDataResidue() const {
	uint32_t residue;
	residue = m_data[8];
	residue = (residue << 8) | m_data[9];
	residue = (residue << 8) | m_data[10];
	residue = (residue << 8) | m_data[11];
	return residue;
}

CommandStatusWrapper::Status CommandStatusWrapper::getStatus() const {
	if (m_data[12] > 0x05) {
		return Status::RESERVED;
	}
	if (m_data[12] >= 0x03) {
		return Status::OBSOLETE;
	}
	return static_cast<Status>(m_data[12]);
}

const ByteBuffer& CommandStatusWrapper::getBuffer() const {
	return m_data;
}

std::ostream& operator<<(std::ostream& os, const CommandStatusWrapper::Status& status) {
	switch (status) {
		case Usbpp::MassStorage::CommandStatusWrapper::Status::PASSED:
			return os << "PASSED (0x00)";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::FAILED:
			return os << "FAILED (0x01)";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::PHASE_ERROR:
			return os << "PHASE ERROR (0x02)";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::OBSOLETE:
			return os << "OBSOLETE";
		case Usbpp::MassStorage::CommandStatusWrapper::Status::RESERVED:
			return os << "RESERVED";
	}

	return os;
}

}
}
