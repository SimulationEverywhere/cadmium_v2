/**
 * Root coordinator for sequential simulation.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */

#ifndef CADMIUM_SIMULATION_ROOT_COORDINATOR_HPP_
#define CADMIUM_SIMULATION_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "core/coordinator.hpp"
#ifndef NO_LOGGING
    #include "logger/logger.hpp"
#endif

namespace cadmium {
    //! Root coordinator class.
    class RootCoordinator {
     protected:
        std::shared_ptr<Coordinator> topCoordinator;  //!< Pointer to top coordinator.
    #ifndef NO_LOGGING
        std::shared_ptr<Logger> logger;               //!< Pointer to simulation logger.
    #endif

        virtual void simulationAdvance(double timeNext) {
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                logger->logTime(timeNext);
            }
        #endif
            topCoordinator->collection(timeNext);
            topCoordinator->transition(timeNext);
            topCoordinator->clear();
        }

     public:
    #ifndef NO_LOGGING
        RootCoordinator(std::shared_ptr<Coupled> model, double time):
            topCoordinator(std::make_shared<Coordinator>(std::move(model), time)), logger() {}
        explicit RootCoordinator(std::shared_ptr<Coupled> model): RootCoordinator(std::move(model), 0) {}
    #else
        RootCoordinator(std::shared_ptr<Coupled> model, double time):
            topCoordinator(std::make_shared<Coordinator>(std::move(model), time)) {}
        explicit RootCoordinator(std::shared_ptr<Coupled> model): RootCoordinator(std::move(model), 0) {}
    #endif
        virtual ~RootCoordinator() = default;

    #ifndef NO_LOGGING
        template <typename T, typename... Args>
        void setLogger(Args&&... args) {
            static_assert(std::is_base_of<Logger, T>::value, "T must inherit cadmium::Logger");
            logger = std::make_shared<T>(std::forward<Args>(args)...);
            topCoordinator->setLogger(logger);
        }

        std::shared_ptr<Logger> getLogger() {
            return logger;
        }
    #endif

        std::shared_ptr<Coordinator> getTopCoordinator() {
            return topCoordinator;
        }

        virtual void start() {
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                logger->start();
            }
        #endif
            topCoordinator->setModelId(0);
            topCoordinator->start(topCoordinator->getTimeLast());
        }

        virtual void stop() {
            topCoordinator->stop(topCoordinator->getTimeLast());
        #ifndef NO_LOGGING
            if (logger != nullptr) {
                logger->stop();
            }
        #endif
        }

        [[maybe_unused]] void simulate(long nIterations) {
            double timeNext = topCoordinator->getTimeNext();
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                this->simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }

        [[maybe_unused]] virtual void simulate(double timeInterval) {
            double timeNext = topCoordinator->getTimeNext();
            double timeFinal = topCoordinator->getTimeLast() + timeInterval;
            while(timeNext < timeFinal) {
                this->simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }
    };
}

#endif // CADMIUM_SIMULATION_SEQUENTIAL_ROOT_COORDINATOR_HPP_
