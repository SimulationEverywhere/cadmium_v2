/**
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2021  Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CADMIUM_BASIC_HPP_
#define CADMIUM_BASIC_HPP_

#include "core/modeling/atomic.hpp"
#include "core/modeling/port.hpp"

namespace cadmium {
	struct Message {
		virtual ~Message() = default;
	};

	class BasicPort : public Port<Message> {};

	struct State {
		virtual ~Message() = default;
	};

	class BasicAtomic: public Atomic<State> {};
}

#endif //CADMIUM_BASIC_HPP_
