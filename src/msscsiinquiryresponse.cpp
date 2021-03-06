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

#include "msscsiinquiryresponse.h"

#include <cassert>
#include <utility>

namespace Usbpp {
namespace MassStorage {
namespace SCSI {

InquiryResponse::InquiryResponse(const ByteBuffer& buffer): m_buffer(buffer) {
	assert(m_buffer.size() >= 36);
}

InquiryResponse::InquiryResponse(const InquiryResponse& other): m_buffer(other.m_buffer) {
	assert(m_buffer.size() >= 36);
}

InquiryResponse::InquiryResponse(InquiryResponse&& other) noexcept
	: m_buffer(std::move(other.m_buffer)) {
	assert(m_buffer.size() >= 36);
}

uint8_t InquiryResponse::getPeripheralQualifier() const {
	return m_buffer.data()[0] >> 5;
}

uint8_t InquiryResponse::getPeripheralDeviceType() const {
	return m_buffer.data()[0] & 0x1F;
}

bool InquiryResponse::getRMB() const {
	return (m_buffer.data()[1] & 0x80) != 0;
}

uint8_t InquiryResponse::getVersion() const {
	return m_buffer.data()[2];
}

bool InquiryResponse::getNORMACA() const {
	return (m_buffer.data()[3] & 0x20) != 0;
}

bool InquiryResponse::getHISUP() const {
	return (m_buffer.data()[3] & 0x10) != 0;
}

uint8_t InquiryResponse::getResponseDataFormat() const {
	return m_buffer.data()[3] & 0xF;
}

uint8_t InquiryResponse::getAdditionalLength() const {
	return m_buffer.data()[4];
}

bool InquiryResponse::getSCCS() const {
	return (m_buffer.data()[5] & 0x80) != 0;
}

bool InquiryResponse::getACC() const {
	return (m_buffer.data()[5] & 0x40) != 0;
}

uint8_t InquiryResponse::getTPGS() const {
	return (m_buffer.data()[5] >> 4) & 0x3;
}

bool InquiryResponse::get3PC() const {
	return (m_buffer.data()[5] & 0x8) != 0;
}

bool InquiryResponse::getProtect() const {
	return (m_buffer.data()[5] & 0x1) != 0;
}

bool InquiryResponse::getBQUE() const {
	return (m_buffer.data()[6] & 0x80) != 0;
}

bool InquiryResponse::getENCSERV() const {
	return (m_buffer.data()[6] & 0x40) != 0;
}

bool InquiryResponse::getMULTIP() const {
	return (m_buffer.data()[6] & 0x10) != 0;
}

bool InquiryResponse::getMCHNGR() const {
	return (m_buffer.data()[6] & 0x8) != 0;
}

bool InquiryResponse::getADDR16() const {
	return (m_buffer.data()[6] & 0x1) != 0;
}

bool InquiryResponse::getWBUS16() const {
	return (m_buffer.data()[7] & 0x20) != 0;
}

bool InquiryResponse::getSYNC() const {
	return (m_buffer.data()[7] & 0x10) != 0;
}

bool InquiryResponse::getLINKED() const {
	return (m_buffer.data()[7] & 0x8) != 0;
}

bool InquiryResponse::getCMDQUE() const {
	return (m_buffer.data()[7] & 0x2) != 0;
}

ByteBuffer InquiryResponse::getVendorIdentification() const {
	return ByteBuffer(m_buffer.data() + 8, 8);
}

ByteBuffer InquiryResponse::getProductIdentification() const {
	return ByteBuffer(m_buffer.data() + 16, 16);
}

ByteBuffer InquiryResponse::getProductRevisionLevel() const {
	return ByteBuffer(m_buffer.data() +  32, 4);
}

/********************************
 * fields that may not be present
 *******************************/

ByteBuffer InquiryResponse::getDriverSerialNumber() const {
	if (m_buffer.size() >= 44) {
		return ByteBuffer(m_buffer.data() + 36, 8);
	}
	return ByteBuffer();
}

ByteBuffer InquiryResponse::getVendorUnique() const {
	if (m_buffer.size() >= 56) {
		return ByteBuffer(m_buffer.data() + 44, 12);
	}
	return ByteBuffer();
}

uint8_t InquiryResponse::getClocking() const {
	if (m_buffer.size() >= 57) {
		return (m_buffer.data()[56] >> 2) & 0x3;
	}
	// TODO: what default value to use???
	return 0;
}

bool InquiryResponse::getQAS() const {
	if (m_buffer.size() >= 57) {
		return (m_buffer.data()[56] & 0x2) != 0;
	}
	// TODO: what default value to use???
	return 0;
}

bool InquiryResponse::getIUS() const {
	if (m_buffer.size() >= 57) {
		return (m_buffer.data()[56] & 0x1) != 0;
	}
	// TODO: what default value to use???
	return 0;
}

ByteBuffer InquiryResponse::getVersionDescriptor(unsigned int descriptor) const {
	if (m_buffer.size() >= 60 + 2*descriptor) {
		return ByteBuffer(m_buffer.data() + 58 + 2*descriptor, 2);
	}
	return ByteBuffer();
}

ByteBuffer InquiryResponse::getVendorSpecific() const {
	if (m_buffer.size() >= 97) {
		return ByteBuffer(m_buffer.data() + 96, m_buffer.size() - 96);
	}
	return ByteBuffer();
}

}
}
}
