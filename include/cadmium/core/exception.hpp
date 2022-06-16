/**
 * Custom exception classes for Cadmium.
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

#ifndef CADMIUM_CORE_EXCEPTION_HPP_
#define CADMIUM_CORE_EXCEPTION_HPP_

#include <exception>

namespace cadmium {
	class CadmiumException : public std::exception {
	 private:
		std::string message{};
	 public:
		explicit CadmiumException(std::string message): std::exception(), message(std::move(message))  {}

		[[nodiscard]] const char* what() const noexcept override {
			return message.c_str();
		}
	};

	class CadmiumModelException: public CadmiumException {
	 public:
		explicit CadmiumModelException(std::string message): CadmiumException(std::move(message)) {}
	};

	class CadmiumSimulationException: public CadmiumException {
	 public:
		explicit CadmiumSimulationException(std::string message): CadmiumException(std::move(message)) {}
	};
}

#endif //CADMIUM_CORE_EXCEPTION_HPP_
