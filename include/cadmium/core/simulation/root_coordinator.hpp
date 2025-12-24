/**
 * <one line to give the program's name and a brief idea of what it does.>
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */


#ifndef CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_
#define CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <utility>
#include <vector>
#include "coordinator.hpp"
#include "../logger/logger.hpp"

namespace cadmium {
	//! Root coordinator class.
    class RootCoordinator {
     protected:
        std::shared_ptr<Coordinator> topCoordinator;  //!< Pointer to top coordinator.
		std::shared_ptr<Logger> logger;               //!< Pointer to simulation logger.

		void simulationAdvance(double timeNext) {
			if (logger != nullptr) {
				logger->lock();  // TODO are locks necessary here? In theory, you should be the only one executing here
				logger->logTime(timeNext);
				logger->unlock();
			}
			topCoordinator->collection(timeNext);
			topCoordinator->transition(timeNext);
			topCoordinator->clear();
		}

     public:
        RootCoordinator(std::shared_ptr<Coupled> model, double time):
			topCoordinator(std::make_shared<Coordinator>(std::move(model), time)), logger() {}
		explicit RootCoordinator(std::shared_ptr<Coupled> model): RootCoordinator(std::move(model), 0) {}

        void setLogger(const std::shared_ptr<Logger>& log) {
			logger = log;
			topCoordinator->setLogger(log);
		}

        std::shared_ptr<Logger> getLogger() {
        	return logger;
        }

        std::shared_ptr<Coordinator> getTopCoordinator() {
			return topCoordinator;
		}

		void start() {
			if (logger != nullptr) {
				logger->start();
			}
			topCoordinator->setModelId(0);
			topCoordinator->start(topCoordinator->getTimeLast());
		}

		void stop() {
			topCoordinator->stop(topCoordinator->getTimeLast());
			if (logger != nullptr) {
				logger->stop();
			}
		}

		[[maybe_unused]] void simulate(long nIterations) {
            // Firsts, we make sure that Mutexes are not activated
            if (logger != nullptr) {
                logger->removeMutex();
            }
			double timeNext = topCoordinator->getTimeNext();
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
				simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }

		[[maybe_unused]] void simulate(double timeInterval) {
            // Firsts, we make sure that Mutexes are not activated
            if (logger != nullptr) {
                logger->removeMutex();
            }
			double timeNext = topCoordinator->getTimeNext();
			double timeFinal = topCoordinator->getTimeLast()+timeInterval;
            while(timeNext < timeFinal) {
				simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }
    };
}

#endif //CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_
