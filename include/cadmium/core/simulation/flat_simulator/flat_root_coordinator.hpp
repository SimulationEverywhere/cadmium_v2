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

#ifndef _CADMIUM_CORE_SIMULATION_FLAT_ROOT_COORDINATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_FLAT_ROOT_COORDINATOR_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "flat_abs_simulator.hpp"
#include "flat_simulator.hpp"
#include "flat_coordinator.hpp"
#include "../../modeling/atomic.hpp"
#include "../../modeling/coupled.hpp"

namespace cadmium {
    class FlatRootCoordinator {
     private:
        FlatCoordinator top_coordinator;

     public:
        root_coordinator(std::shared_ptr<Coupled> model) {
        	top_coordinator(model);
        }

		void start() {
            top_coordinator.start();
		}

		void stop() {
			top_coordinator.stop();
		}

		void setLogger(const std::shared_ptr<Logger>& log) {
			top_coordinator.setLogger(log);
		}

        void simulate(long nIterations) {
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                timeLast = timeNext;
                top_coordinator.execute_output_functions(timeNext);
                top_coordinator.route_messages();
                timeNext = top_coordinator.execute_state_transitions(timeNext);
                top_coordinator.clear();
            }
        }

		void simulate(double timeInterval) {
            double timeFinal = timeLast + timeInterval;
            while(timeNext < timeFinal) {
                timeLast = timeNext;
                top_coordinator.execute_output_functions(timeNext);
                top_coordinator.route_messages();
                timeNext = top_coordinator.execute_state_transitions(timeNext);
                top_coordinator.clear();
            }
            timeLast = timeFinal;
        }
    };
}

#endif //_CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_
