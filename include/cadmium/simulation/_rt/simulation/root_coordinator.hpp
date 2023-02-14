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

#ifndef RT_EXECUTE
#define RT_EXECUTE
#endif

#ifndef CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_
#define CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <utility>
#include <vector>
#include "coordinator.hpp"

#ifndef RT_ARM_MBED
#include "../logger/logger.hpp"
#endif

#include "cadmium/simulation/rt/real_time/rt_clock.hpp"

namespace cadmium {
    //! Root coordinator class.
    class RootCoordinator {
     protected:
        std::shared_ptr<Coordinator> topCoordinator;  //!< Pointer to top coordinator.
#ifndef RT_ARM_MBED
        std::shared_ptr<Logger> logger;               //!< Pointer to simulation logger.
#else
        RTClock timmer;
#endif

        void simulationAdvance(double timeNext) {
#ifndef RT_ARM_MBED
            if (logger != nullptr) {
                logger->lock();
                logger->logTime(timeNext);
                logger->unlock();
            }
#endif
            topCoordinator->collection(timeNext);
            topCoordinator->transition(timeNext);
            topCoordinator->clear();
        }

     public:
#ifndef RT_ARM_MBED
        RootCoordinator(std::shared_ptr<Coupled> model, double time):
            topCoordinator(std::make_shared<Coordinator>(std::move(model), time)), logger() {}

        explicit RootCoordinator(std::shared_ptr<Coupled> model): RootCoordinator(std::move(model), 0) {}
#else
        RootCoordinator(std::shared_ptr<Coupled> model, double time):
				topCoordinator(std::make_shared<Coordinator>(std::move(model), time)),timmer(topCoordinator->get_async_subjects()) {}
        
			explicit RootCoordinator(std::shared_ptr<Coupled> model): RootCoordinator(std::move(model), 0) {}
#endif

        std::shared_ptr<Coordinator> getTopCoordinator() {
            return topCoordinator;
        }

#ifndef RT_ARM_MBED
        std::shared_ptr<Logger> getLogger() {
            return logger;
        }
        void setLogger(const std::shared_ptr<Logger>& log) {
            logger = log;
            topCoordinator->setLogger(log);
        }
#endif

        void start() {
#ifndef RT_ARM_MBED
            if (logger != nullptr) {
                logger->start();
            }
#endif
            topCoordinator->setModelId(0);
            topCoordinator->start(topCoordinator->getTimeLast());
        }

        void stop() {
            topCoordinator->stop(topCoordinator->getTimeLast());
#ifndef RT_ARM_MBED
            if (logger != nullptr) {
                logger->stop();
            }
#endif
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

#ifdef RT_ARM_MBED
        void simulateRT(double timeInterval){
        double timeNext = topCoordinator->getTimeNext(); // the time of the first known event
        timmer.startSimulation(); // reset RT clock to be at 0 seconds
        double currentTime = 0;
        double e;
        while(1) {
            e = timmer.wait_for(timeNext - currentTime);
            if(!timmer.interrupted && e == 0){ // if no interrupt occured
                currentTime = timeNext;
                simulationAdvance(currentTime);	
            }else{ // There was an interupt
                currentTime += e;
                for(auto s : timmer.getAsyncSubjects()){
                    timmer.interrupted = false;
                    if(s->interrupted){
                        s->interrupted = false;
                        topCoordinator->inject(e, s->getPort(), true); // insert a message into the right port triggering external event
                    }
                }
            }
            timeNext = topCoordinator->getTimeNext();
        }
    }
#endif

        [[maybe_unused]] void simulate(double timeInterval) {
#ifdef RT_ARM_MBED
            simulateRT(timeInterval);
#else
            // Firsts, we make sure that Mutexes are not activated
            if (logger != nullptr) {
                logger->removeMutex();
            }
            double timeNext = topCoordinator->getTimeNext();
            double timeFinal = topCoordinator->getTimeLast() + timeInterval;
            while (timeNext < timeFinal) {
                simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
#endif
        }
    };
}

#endif //CADMIUM_CORE_SIMULATION_ROOT_COORDINATOR_HPP_
