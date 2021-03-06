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

#ifndef LIBUSBPP_MASS_CSW_H_
#define LIBUSBPP_MASS_CSW_H_

#include "buffer.h"

#include <cstdint>
#include <ostream>
#include <vector>

namespace Usbpp {
namespace MassStorage {

class CommandStatusWrapper {
public:
	enum class Status : uint8_t {
		PASSED = 0x00,
		FAILED = 0x01,
		PHASE_ERROR = 0x02,
		OBSOLETE, // this should never happen
		RESERVED // this should never happen
	};

	CommandStatusWrapper();
	explicit CommandStatusWrapper(const ByteBuffer& buffer);
	CommandStatusWrapper(uint32_t dCSWTag, uint32_t dCSWDataResidue, uint8_t bCSWStatus);
	virtual ~CommandStatusWrapper();

	CommandStatusWrapper(const CommandStatusWrapper& other);
	CommandStatusWrapper(CommandStatusWrapper&& other) noexcept;
	CommandStatusWrapper& operator=(const CommandStatusWrapper& other);
	CommandStatusWrapper& operator=(CommandStatusWrapper&& other) noexcept;

	uint32_t getTag() const;
	uint32_t getDataResidue() const;
	Status getStatus() const;

	const ByteBuffer& getBuffer() const;

private:
	ByteBuffer m_data;
};

std::ostream& operator<<(std::ostream& os, const CommandStatusWrapper::Status& status);

}
}

#endif
