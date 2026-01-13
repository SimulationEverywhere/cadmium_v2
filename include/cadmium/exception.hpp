/**
 * Custom exception classes for Cadmium.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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
