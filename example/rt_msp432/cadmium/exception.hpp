/**
 * Custom exception classes for Cadmium.
 * Copyright (C) 2022  Román Cárdenas Rodríguez
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

#ifndef CADMIUM_EXCEPTION_HPP_
#define CADMIUM_EXCEPTION_HPP_

#include <exception>
#include <string>

namespace cadmium {
	//! Base class for exceptions thrown by Cadmium.
	class CadmiumException : public std::exception {
	 private:
		std::string message{};  //!< Message with information about what caused the exception.
	 public:
		/**
		 * Constructor function
		 * @param message information about the cause of the exception.
		 */
		explicit CadmiumException(std::string message): std::exception(), message(std::move(message))  {}

		//! @return pointer to the exception message.
		[[nodiscard]] const char* what() const noexcept override {
			return message.c_str();
		}
	};

	//! Exceptions thrown due to an error in the model.
	class CadmiumModelException: public CadmiumException {
	 public:
		/**
		 * Constructor function
		 * @param message information about the modeling error.
		 */
		explicit CadmiumModelException(std::string message): CadmiumException(std::move(message)) {}
	};

	//! Exceptions thrown due to an error in the simulation.
	class CadmiumSimulationException: public CadmiumException {
	 public:
		/**
		 * Constructor function
		 * @param message information about the simulation error.
		 */
		explicit CadmiumSimulationException(std::string message): CadmiumException(std::move(message)) {}
	};

    //! Exceptions thrown due to an error in a real-time clock.
    class CadmiumRTClockException: public CadmiumException {
     public:
        /**
         * Constructor function
         * @param message information about the simulation error.
         */
        explicit CadmiumRTClockException(std::string message): CadmiumException(std::move(message)) {}
    };
}

#endif //CADMIUM_EXCEPTION_HPP_
