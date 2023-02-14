/**
 * Root coordinator for sequential simulation.
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

#ifndef CADMIUM_SIMULATION_ROOT_COORDINATOR_HPP_
#define CADMIUM_SIMULATION_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "core/coordinator.hpp"
#include "logger/logger.hpp"

namespace cadmium {
    //! Root coordinator class.
    class RootCoordinator {
     protected:
        std::shared_ptr<Coordinator> topCoordinator;  //!< Pointer to top coordinator.
        std::shared_ptr<Logger> logger;               //!< Pointer to simulation logger.

        virtual void simulationAdvance(double timeNext) {
            if (logger != nullptr) {
                logger->logTime(timeNext);
            }
            topCoordinator->collection(timeNext);
            topCoordinator->transition(timeNext);
            topCoordinator->clear();
        }

     public:
        RootCoordinator(std::shared_ptr<Coupled> model, double time):
            topCoordinator(std::make_shared<Coordinator>(std::move(model), time)), logger() {}
        explicit RootCoordinator(std::shared_ptr<Coupled> model): RootCoordinator(std::move(model), 0) {}

        virtual ~RootCoordinator() = default;

        template <typename T, typename... Args>
        void setLogger(Args&&... args) {
            static_assert(std::is_base_of<Logger, T>::value, "T must inherit cadmium::Logger");
            logger = std::make_shared<T>(std::forward<Args>(args)...);
            topCoordinator->setLogger(logger);
        }

        std::shared_ptr<Logger> getLogger() {
            return logger;
        }

        std::shared_ptr<Coordinator> getTopCoordinator() {
            return topCoordinator;
        }

        virtual void start() {
            if (logger != nullptr) {
                logger->start();
            }
            topCoordinator->setModelId(0);
            topCoordinator->start(topCoordinator->getTimeLast());
        }

        virtual void stop() {
            topCoordinator->stop(topCoordinator->getTimeLast());
            if (logger != nullptr) {
                logger->stop();
            }
        }

        [[maybe_unused]] void simulate(long nIterations) {
            double timeNext = topCoordinator->getTimeNext();
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                this->simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }

        [[maybe_unused]] void simulate(double timeInterval) {
            double timeNext = topCoordinator->getTimeNext();
            double timeFinal = topCoordinator->getTimeLast()+timeInterval;
            while(timeNext < timeFinal) {
                this->simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }
    };
}

#endif // CADMIUM_SIMULATION_SEQUENTIAL_ROOT_COORDINATOR_HPP_
