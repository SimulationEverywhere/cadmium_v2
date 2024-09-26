/**
 * CSV logger.
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

#ifndef CADMIUM_CORE_LOGGER_RT_LOGGER_HPP_
#define CADMIUM_CORE_LOGGER_RT_LOGGER_HPP_

#include <iostream>
#include <string>
#include <utility>
#include "logger.hpp"

namespace cadmium {
	//! Cadmium CSV logger class.
	class RTLogger: public Logger {
	 private:
		std::string sep;       //!< String used as column separation. 
 		 // std::ostream& sink;    //!< output stream.
	 public:
		/**
		 * Constructor function.
		 * @param filepath path to the CSV file.
		 * @param sep string used as column separation.
		 */
		RTLogger(std::string sep): Logger(), sep(std::move(sep)){}

		/**
		 * Constructor function. Separation is set to ",".
		 * @param filepath path to the CSV file.
		 */
		explicit RTLogger(): RTLogger(",") {}

		//! It starts the output file stream and prints the CSV header.
		void start() override {
			std::cout << "time" << sep << "model_id" << sep << "model_name" << sep << "port_name" << sep << "data" << std::endl;
		}

		//! It closes the output file after the simulation.
		void stop() override {
		}

		/**
		 * Virtual method to log atomic models' output messages.
		 * @param time current simulation time.
		 * @param modelId ID of the model that generated the output message.
		 * @param modelName name of the model that generated the output message.
		 * @param portName name of the model port in which the output message was created.
		 * @param output string representation of the output message.
		 */
		void logOutput(double time, long modelId, const std::string& modelName, const std::string& portName, const std::string& output) override {
			std::cout << time << sep << modelId << sep << modelName << sep << portName << sep << output << std::endl;
		}

		/**
		 * Virtual method to log atomic models' states.
		 * @param time current simulation time.
		 * @param modelId ID of the model that generated the output message.
		 * @param modelName name of the model that generated the output message.
		 * @param state string representation of the state.
		 */
		void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {
			std::cout << time << sep << modelId << sep << modelName << sep << sep << state << std::endl;
		}
	};
}

#endif //CADMIUM_CORE_LOGGER_RT_LOGGER_HPP_
