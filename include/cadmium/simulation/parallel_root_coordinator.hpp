/**
 * Coordinator for executing simulations in parallel.
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

#ifndef CADMIUM_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
#define CADMIUM_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <omp.h>
#include <thread>
#include <utility>
#include <vector>
#include "root_coordinator.hpp"
#ifndef NO_LOGGING
    #include "logger/logger.hpp"
    #include "logger/mutex.hpp"
#endif

namespace cadmium {
    //! Parallel Root coordinator class.
    class ParallelRootCoordinator {
     private:
        std::shared_ptr<RootCoordinator> rootCoordinator;
        //! It serializes the IC couplings in pairs <port_to, {ports_from}> to parallelize message propagation.
        std::vector<std::pair<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>>> stackedIC;
     public:
        ParallelRootCoordinator(std::shared_ptr<Coupled> model, double time) {
            model->flatten();  // In parallel execution, models MUST be flat
            rootCoordinator = std::make_shared<RootCoordinator>(model, time);
            for (const auto& [portTo, portsFrom]: model->getICs()) {
                stackedIC.emplace_back(portTo, portsFrom);
            }
        }
        explicit ParallelRootCoordinator(std::shared_ptr<Coupled> model): ParallelRootCoordinator(std::move(model), 0) {}


    #ifndef NO_LOGGING
        template <typename T, typename... Args>
        void setLogger(Args&&... args) {
            T logger = T(std::forward<Args>(args)...);
            rootCoordinator->setLogger<MutexLogger<T>>(std::move(logger));
        }
    #endif

        void start() {
            rootCoordinator->start();
        }

        void stop() {
            rootCoordinator->stop();
        }

        void simulate(long nIterations, size_t thread_number = std::thread::hardware_concurrency()) {
            double timeNext = rootCoordinator->getTopCoordinator()->getTimeNext();
            // Threads created
#pragma omp parallel default(none) num_threads(thread_number) shared(timeNext, nIterations)
            {
                //each thread get its if within the group
                size_t tid = omp_get_thread_num();

                auto subcomponents = rootCoordinator->getTopCoordinator()->getSubcomponents();
                auto nSubcomponents = subcomponents.size();
                auto nICs = stackedIC.size();
                double localNext;

                while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                    // Step 1: execute output functions
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nSubcomponents; i++) {
                        subcomponents.at(i)->collection(timeNext);
                    }
#pragma omp barrier
                    //end Step 1

                    // Step 2: route messages
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nICs; i++) {  // We only parallelize by destination port, right?
                        for (auto& portFrom: stackedIC[i].second) {
                            stackedIC.at(i).first->propagate(portFrom);
                        }
                    }
#pragma omp barrier
                    // end Step 2

                    // Step 3: state transitions
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nSubcomponents; i++) {
                        subcomponents.at(i)->transition(timeNext);
                        subcomponents.at(i)->clear();
                    }
#pragma omp barrier
                    // end Step 3

                    // Step 4: time for next events
                    localNext = subcomponents[0]->getTimeNext();  // Potential bug: what if model is empty? I'd initialize this to infinity and iterate from 0
#pragma omp for schedule(static)
                    for (size_t i = 1; i < nSubcomponents; i++){
                        if (subcomponents[i]->getTimeNext() < localNext) {
                            localNext = subcomponents[i]->getTimeNext();
                        }
                    }
#pragma omp single
                    {
                        timeNext = localNext;
                    }
#pragma omp barrier
#pragma omp critical
                    {
                        if (localNext < timeNext) {
                            timeNext = localNext;
                        }
                    }
#pragma omp barrier
                    //end Step 4

                }//end simulation loop
            }
        }

        void simulate(double timeInterval, size_t thread_number = std::thread::hardware_concurrency()) {
            double timeNext = rootCoordinator->getTopCoordinator()->getTimeNext();
            double timeFinal = rootCoordinator->getTopCoordinator()->getTimeLast()+timeInterval;

            //threads created
#pragma omp parallel default(none) num_threads(thread_number) shared(timeNext, timeFinal, rootCoordinator)
            {
                //each thread get its if within the group
                size_t tid = omp_get_thread_num();

                auto& subcomponents = rootCoordinator->getTopCoordinator()->getSubcomponents();
                auto nSubcomponents = subcomponents.size();
                auto nICs = stackedIC.size();
                double localNext;

                while(timeNext < timeFinal) {
                    // Step 1: execute output functions
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nSubcomponents; i++) {
                        subcomponents.at(i)->collection(timeNext);
                    }
#pragma omp barrier
                    //end Step 1

                    // Step 2: route messages
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nICs; i++) {
                        for (auto& portFrom: stackedIC[i].second){
                            stackedIC.at(i).first->propagate(portFrom);
                        }
                    }
#pragma omp barrier
                    // end Step 2

                    // Step 3: state transitions
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nSubcomponents; i++){
                        subcomponents.at(i)->transition(timeNext);
                        subcomponents.at(i)->clear();
                    }
#pragma omp barrier
                    // end Step 3

                    // Step 4: time for next events
                    localNext = subcomponents[0]->getTimeNext();
#pragma omp for schedule(static)
                    for (size_t i = 1; i < nSubcomponents; i++){
                        if (subcomponents[i]->getTimeNext() < localNext){
                            localNext = subcomponents[i]->getTimeNext();
                        }
                    }
#pragma omp single
                    {
                        timeNext = localNext;
                    }
#pragma omp barrier
#pragma omp critical
                    {
                        if (localNext < timeNext) {
                            timeNext = localNext;
                        }
                    }
#pragma omp barrier
                    //end Step 4

                }//end simulation loop
            }
        }

        void simulateSerialCollection(double timeInterval, size_t thread_number = std::thread::hardware_concurrency()) {
            double timeNext = rootCoordinator->getTopCoordinator()->getTimeNext();
            double timeFinal = rootCoordinator->getTopCoordinator()->getTimeLast() + timeInterval;

            //threads created
#pragma omp parallel default(none) num_threads(thread_number) shared(timeNext, timeFinal, rootCoordinator)
            {
                //each thread get its if within the group
                size_t tid = omp_get_thread_num();

                auto& subcomponents = rootCoordinator->getTopCoordinator()->getSubcomponents();
                auto nSubcomponents = subcomponents.size();
                auto nICs = stackedIC.size();
                double localNext;

                while (timeNext < timeFinal) {
                    // Step 1: execute output functions
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nSubcomponents; i++){
                        subcomponents.at(i)->collection(timeNext);
                    }
#pragma omp barrier
                    //end Step 1

                    // Step 2: route messages (in sequential)
                    for (const auto& [portTo, portsFrom]: stackedIC) {
                        for (const auto& portFrom: portsFrom) {
                            portTo->propagate(portFrom);
                        }
                    }
                    // end Step 2

                    // Step 3: state transitions
#pragma omp for schedule(static)
                    for (size_t i = 0; i < nSubcomponents; i++) {
                        subcomponents.at(i)->transition(timeNext);
                        subcomponents.at(i)->clear();
                    }
#pragma omp barrier
                    // end Step 3

                    // Step 4: time for next events
                    localNext = subcomponents[0]->getTimeNext();
#pragma omp for schedule(static)
                    for (size_t i = 1; i < nSubcomponents; i++){
                        if (subcomponents[i]->getTimeNext() < localNext) {
                            localNext = subcomponents[i]->getTimeNext();
                        }
                    }
#pragma omp single
                    {
                        timeNext = localNext;
                    }
#pragma omp barrier
#pragma omp critical
                    {
                        if (localNext < timeNext) {
                            timeNext = localNext;
                        }
                    }
#pragma omp barrier
                    //end Step 4

                }//end simulation loop
            }
        }
    };
}

#endif // CADMIUM_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
