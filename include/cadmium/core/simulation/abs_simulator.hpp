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

#ifndef _CADMIUM_CORE_SIMULATION_ABS_SIMULATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_ABS_SIMULATOR_HPP_

#include <limits>
#include <memory>
#include "../logger/logger.hpp"
#include "../modeling/component.hpp"

namespace cadmium {
    class AbstractSimulator {
	 protected:
		long modelId;
        double timeLast, timeNext;
	 public:
		explicit AbstractSimulator(double time): modelId(), timeLast(time), timeNext(std::numeric_limits<double>::infinity()) {}
		virtual ~AbstractSimulator() = default;

		[[nodiscard]] double getTimeLast() const {
			return timeLast;
		}
		[[nodiscard]] double getTimeNext() const {
			return timeNext;
		}

		[[nodiscard]] virtual std::shared_ptr<Component> getComponent() const = 0;
		virtual long setModelId(long next) = 0;
		virtual void setLogger(const std::shared_ptr<Logger>& log) = 0;
		virtual void setDebugLogger(const std::shared_ptr<Logger>& log) = 0;
		virtual void start(double time) = 0;
		virtual void stop(double time) = 0;
		virtual void collection(double time) = 0;
		virtual void transition(double time) = 0;
		virtual void clear() = 0;
    };
}

#endif //_CADMIUM_CORE_SIMULATION_ABS_SIMULATOR_HPP_
