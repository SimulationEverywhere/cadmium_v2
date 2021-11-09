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

#ifndef _CADMIUM_CORE_LOGGER_LOGGER_HPP_
#define _CADMIUM_CORE_LOGGER_LOGGER_HPP_

#include <mutex>
#include <string>

namespace cadmium {
	class Logger {
	 private:
		std::mutex mutex;
	 public:
		Logger(): mutex() {}
		virtual ~Logger() = default;

		inline void lock() {
			mutex.lock();
		}

		inline void unlock() {
			mutex.unlock();
		}

		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void logOutput(double time, long modelId, const std::string& modelName, const std::string& portName, const std::string& output) = 0;
		virtual void logState(double time, long modelId, const std::string& modelName, const std::string& state) = 0;
	};
}

#endif //_CADMIUM_CORE_LOGGER_LOGGER_HPP_
