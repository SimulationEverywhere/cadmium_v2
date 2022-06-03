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

#include <limits>
#include <memory>
#include <utility>
#include <vector>
#include "coordinator.hpp"
#include "../logger/logger.hpp"

namespace cadmium {
    class RootCoordinator {
     private:
        std::shared_ptr<Coordinator> topCoordinator;
		std::shared_ptr<Logger> logger, debugLogger;

		void simulationAdvance(double timeNext) {
			if (logger != nullptr) {
				logger->lock();
				logger->logTime(timeNext);
				logger->unlock();
			}
			if (debugLogger != nullptr) {
				debugLogger->lock();
				debugLogger->logTime(timeNext);
				debugLogger->unlock();
			}
			topCoordinator->collection(timeNext);
			topCoordinator->transition(timeNext);
			topCoordinator->clear();
		}
     public:
        RootCoordinator(std::shared_ptr<Coupled> model, double time):
		  topCoordinator(std::make_shared<Coordinator>(std::move(model), time)), logger(), debugLogger() {
		}
		explicit RootCoordinator(std::shared_ptr<Coupled> model): RootCoordinator(std::move(model), 0) {}
		explicit RootCoordinator(Coupled model) : RootCoordinator(std::make_shared<Coupled>(std::move(model))) {}

		void start() {
			if (logger != nullptr) {
				logger->start();
			}
			if (debugLogger != nullptr) {
				debugLogger->start();
			}
		    topCoordinator->start();
		}

		void stop() {
			topCoordinator->stop();
			if (logger != nullptr) {
				logger->stop();
			}
			if (debugLogger != nullptr) {
				debugLogger->stop();
			}
		}

		void setLogger(const std::shared_ptr<Logger>& log) {
			logger = log;
            topCoordinator->setLogger(log);
		}

		void setDebugLogger(const std::shared_ptr<Logger>& log) {
			debugLogger = log;
			topCoordinator->setDebugLogger(log);
		}

		std::shared_ptr<Coordinator> getTopCoordinator() {
			return topCoordinator;
		}

		[[maybe_unused]] void simulate(long nIterations) {
			double timeNext = topCoordinator->getTimeNext();
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
				simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }

		[[maybe_unused]] void simulate(double timeInterval) {
			double timeNext = topCoordinator->getTimeNext();
			double timeFinal = topCoordinator->getTimeLast()+timeInterval;
            while(timeNext < timeFinal) {
				simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }
    };
}

#endif //_CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_
