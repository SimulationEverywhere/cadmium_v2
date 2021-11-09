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

#ifndef _CADMIUM_BASIC_HPP_
#define _CADMIUM_BASIC_HPP_

#include <iostream>
#include <memory>
#include <utility>
#include "core/modeling/atomic.hpp"
#include "core/modeling/port.hpp"

namespace cadmium {
	struct Message {
		virtual ~Message() = default;
		virtual const std::string str() const = 0;
	};

 	class BasicPort : public Port<std::unique_ptr<Message>> {
	  public:
		template <typename T>
		void addMessage(const T message) {
			Port<std::unique_ptr<Message>>::addMessage(std::make_unique<T>(std::move(message)));
		}

		template <typename T>
		static void addMessage(const std::shared_ptr<PortInterface>& port, const T message) {
			Port<std::unique_ptr<Message>>::addMessage(port, std::make_unique<T>(std::move(message)));
		}
	};

	std::ostream& operator<<(std::ostream &out, const std::unique_ptr<Message>& m) {
		out << m->str();
		return out;
	}

	struct State {
		virtual ~State() = default;
		virtual const std::string str() const = 0;
	};

 	class BasicAtomic: public Atomic<std::unique_ptr<State>> {
	 public:
		template <typename S>
		BasicAtomic(S initialState) : Atomic<std::unique_ptr<State>>(std::make_unique<S>(std::move(initialState))) {}
	};

	std::ostream& operator<<(std::ostream &out, const std::unique_ptr<State>& s) {
		out << s->str();
		return out;
	}
}

#endif //_CADMIUM_BASIC_HPP_
