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

#ifndef _CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "coordinator.hpp"

namespace cadmium {
    class RootCoordinator {
     private:
        std::shared_ptr<Coordinator> top_coordinator;
        double timeLast,timeNext;

     public:
        RootCoordinator(std::shared_ptr<Coupled> model, double initialTime) {
            top_coordinator = std::make_shared<Coordinator>(Coordinator(model, initialTime));
            timeLast = initialTime;
            timeNext = initialTime;
        }

        explicit RootCoordinator(std::shared_ptr<Coupled> model) : RootCoordinator(model, 0) {}

        explicit RootCoordinator(Coupled model) : RootCoordinator(std::make_shared<Coupled>(std::move(model)), 0) {}


		void start() {
		    top_coordinator->start();
		}

		void stop() {
			top_coordinator->stop();
		}

		void setLogger(const std::shared_ptr<Logger>& log) {
            top_coordinator->setLogger(log);
		}

		void setDebugLogger(const std::shared_ptr<Logger>& log) {
			top_coordinator->setDebugLogger(log);
		}

		std::shared_ptr<Coordinator> getTopCoordinator() {
			return top_coordinator;
		}

        void simulate(long nIterations) {
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                if (top_coordinator->getLogger() != nullptr) {
					top_coordinator->getLogger()->logTime(timeNext);
				}
				if (top_coordinator->getDebugLogger() != nullptr) {
					top_coordinator->getDebugLogger()->logTime(timeNext);
				}
                top_coordinator->collection(timeNext);
                top_coordinator->transition(timeNext);
                top_coordinator->clear();
                timeNext = top_coordinator->getTimeNext();
            }
        }

		void simulate(double timeInterval) {
			double timeFinal = top_coordinator->getTimeLast()+timeInterval;
            while(timeNext < timeFinal) {
                if (top_coordinator->getLogger() != nullptr) {
					top_coordinator->getLogger()->logTime(timeNext);
				}
				if (top_coordinator->getDebugLogger() != nullptr) {
					top_coordinator->getDebugLogger()->logTime(timeNext);
				}
                top_coordinator->collection(timeNext);
                top_coordinator->transition(timeNext);
                top_coordinator->clear();
                timeNext = top_coordinator->getTimeNext();
            }
        }

    };
}

#endif //_CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_
