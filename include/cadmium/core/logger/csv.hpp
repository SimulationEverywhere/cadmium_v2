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

#ifndef _CADMIUM_CORE_LOGGER_CSV_LOGGER_HPP_
#define _CADMIUM_CORE_LOGGER_CSV_LOGGER_HPP_

#include <fstream>
#include <string>
#include <utility>
#include "logger.hpp"

namespace cadmium {
	class CSVLogger: public Logger {
	 private:
		std::string filepath;
		std::string sep;
		std::ofstream file;
	 public:
		CSVLogger(std::string filepath, std::string sep): Logger(), filepath(std::move(filepath)), sep(std::move(sep)), file() {}
		explicit CSVLogger(std::string filepath): CSVLogger(std::move(filepath), ",") {}

		void start() override {
			file.open(filepath);
			file << "time" << sep << "model_id" << sep << "model_name" << sep << "port_name" << sep << "data" << std::endl;
		}

		void stop() override {
			file.close();
		}

		void logOutput(double time, long modelId, const std::string& modelName, const std::string& portName, const std::string& output) override {
			file << time << sep << modelId << sep << modelName << sep << portName << sep << output << std::endl;
		}

		void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {
			file << time << sep << modelId << sep << modelName << sep << sep << state << std::endl;
		}
	};
}

#endif //_CADMIUM_CORE_LOGGER_CSV_LOGGER_HPP_
