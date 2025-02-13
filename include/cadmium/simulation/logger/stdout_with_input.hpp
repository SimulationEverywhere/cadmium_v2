/**
 * STDOUT logger.
 * Copyright (C) 2023 Ezequiel Pecker Marcosig 
 * ARSLab - Carleton University
 * SEDLab - University of Buenos Aires
 * Modified by: Sasisekhar Govind
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PASTDOUTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CADMIUM_CORE_LOGGER_STDOUT_LOGGER_HPP_ //STDOUT and this should not be defined together
#define CADMIUM_CORE_LOGGER_STDOUT_LOGGER_HPP_

#include <iostream>
#include <string>
#include <utility>
#include "logger.hpp"

namespace cadmium {
	//! Cadmium STDOUT_with_input logger class. (same as stdout)
	class STDOUTLogger: public Logger {
	 private:
		std::string sep;       //!< String used as column separation. 
 		 // std::ostream& sink;    //!< output stream.
	 public:
		/**
		 * Constructor function.
		 * @param sep string used as column separation.
		 */
		STDOUTLogger(std::string sep): Logger(), sep(std::move(sep)){}

		/**
		 * Constructor function. Separation is set to ",".
		 */
		explicit STDOUTLogger(): STDOUTLogger(",") {}

		//! It starts the output file stream and prints the header in standard output.
		void start() override {
			std::cout << "time" << sep << "model_id" << sep << "model_name" << sep << "port_name" << sep << "data" << std::endl;
		}

		//! It does nothing after the simulation.
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
			std::cout << "\x1B[32m" << time << sep << modelId << sep << modelName << sep << portName << sep << output << "\033[0m" << std::endl;
		}

		/**
		 * Virtual method to log atomic models' states.
		 * @param time current simulation time.
		 * @param modelId ID of the model that generated the output message.
		 * @param modelName name of the model that generated the output message.
		 * @param state string representation of the state.
		 */
		void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {

#ifdef NO_LOG_STATE
			//Do not output anything...maybe add std::cout << "del int"?
#else
			std::cout << "\x1B[33m" << time << sep << modelId << sep << modelName << sep << sep << state << "\033[0m" << std::endl;
#endif
		}

		virtual void logModel(double time,
            long modelId,
            const std::shared_ptr<AtomicInterface>& model,
            bool logOutput) {
            if (logOutput) {
                for (const auto& outPort: model->getOutPorts()) {
                    for (std::size_t i = 0; i < outPort->size(); ++i) {
                        this->logOutput(time, modelId, model->getId(), outPort->getId(), outPort->logMessage(i));
                    }
                }
            }
			for (const auto& outPort: model->getInPorts()) {
				for (std::size_t i = 0; i < outPort->size(); ++i) {
					this->logOutput(time, modelId, model->getId(), outPort->getId(), outPort->logMessage(i));
				}
			}
            this->logState(time, modelId, model->getId(), model->logState());
        }
	};
}

#endif //CADMIUM_CORE_LOGGER_STDOUT_LOGGER_HPP_
