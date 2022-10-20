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

#ifndef CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
#define CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <omp.h>
#include <thread>
#include <utility>
#include <vector>
#include "root_coordinator.hpp"
#include "../logger/logger.hpp"

#include <iostream>

namespace cadmium {
    //! Parallel Root coordinator class.
    class ParallelRootCoordinator: public RootCoordinator {
     private:
        //! It serializes the IC couplings in pairs <port_to, {ports_from}> to parallelize message propagation.
        std::vector<std::pair<std::shared_ptr<PortInterface>, std::vector<std::shared_ptr<PortInterface>>>> stackedIC;
     public:
        ParallelRootCoordinator(std::shared_ptr<Coupled> model, double time): RootCoordinator(std::move(model), time), stackedIC() {
            auto coupled = topCoordinator->getCoupled();
            coupled->flatten();  // In parallel execution, models MUST be flat

            for (const auto& [portTo, portsFrom]: coupled->getICs()) {
                stackedIC.emplace_back(portTo, portsFrom);
            }
        }
        explicit ParallelRootCoordinator(std::shared_ptr<Coupled> model): ParallelRootCoordinator(std::move(model), 0) {}

        void simulate(long nIterations, size_t thread_number = std::thread::hardware_concurrency()) {
            double timeNext = topCoordinator->getTimeNext();

            // Threads created
			#pragma omp parallel default(none) num_threads(thread_number) shared(timeNext, nIterations)
            {
                //each thread get its if within the group
                size_t tid = omp_get_thread_num();
                //get list of subcomponents
                auto subcomponents = topCoordinator->getSubcomponents();
                //get number of subcomponents
                auto n_subcomponents = subcomponents.size();
                //get number of internal couplings
                auto n_internal_couplings = stackedIC.size();
                double localNext;

                while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
                    // Step 1: execute output functions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                        subcomponents.at(i)->collection(timeNext);
                    }
					#pragma omp barrier
                    //end Step 1

            		// Step 2: route messages
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_internal_couplings;i++){
                    	for(auto& portFrom: stackedIC[i].second){
                            stackedIC.at(i).first->propagate(portFrom);
                    	}
                    }
					#pragma omp barrier
                    // end Step 2

                    // Step 3: state transitions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                        subcomponents.at(i)->transition(timeNext);
                        subcomponents.at(i)->clear();
                    }
					#pragma omp barrier
                    // end Step 3

                    // Step 4: time for next events
                    localNext = subcomponents[0]->getTimeNext();
					#pragma omp for schedule(static)
                    for(size_t i=1; i<n_subcomponents;i++){
                        if(subcomponents[i]->getTimeNext() < localNext){
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
                        if(localNext < timeNext){
                            timeNext = localNext;
                        }
                    }
					#pragma omp barrier
                    //end Step 4

                }//end simulation loop
            }
        }

        // TODO what is the difference between simulate, simulate_serial, and simulate_stacked?
        void simulate(double timeInterval, size_t thread_number = std::thread::hardware_concurrency()) {
        	double timeNext = topCoordinator->getTimeNext();
            double timeFinal = topCoordinator->getTimeLast()+timeInterval;

            //threads created
			#pragma omp parallel default(none) num_threads(thread_number) shared(timeNext, timeFinal, topCoordinator)
            {
                //each thread get its if within the group
                size_t tid = omp_get_thread_num();
                //get list of subcomponents
                auto& subcomponents = topCoordinator->getSubcomponents();
                //get number of subcomponents
                auto n_subcomponents = subcomponents.size();
                //auto& internal_couplings = topCoordinator->getSerialIC();
                //get number of internal couplings
                auto n_internal_couplings = stackedIC.size();
                double localNext;

                while(timeNext < timeFinal) {
                    // Step 1: execute output functions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                    	subcomponents.at(i)->collection(timeNext);
                    }
					#pragma omp barrier
                    //end Step 1

/*
					// Step 2: route messages
					#pragma omp for schedule(static)
					for(size_t i=0; i<n_internal_couplings;i++) {
                    	internal_couplings.at(i).first->parallelPropagate(internal_couplings.at(i).second);
                    }
                    // end Step 2
*/
            		// Step 2: route messages
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_internal_couplings;i++){
                    	for(auto& portFrom: stackedIC[i].second){
                            stackedIC.at(i).first->propagate(portFrom);
                    	}
                    }
					#pragma omp barrier
                    // end Step 2

                    // Step 3: state transitions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                        subcomponents.at(i)->transition(timeNext);
                        subcomponents.at(i)->clear();
                    }
					#pragma omp barrier
                    // end Step 3

                    // Step 4: time for next events
                    localNext = subcomponents[0]->getTimeNext();
					#pragma omp for schedule(static)
                    for(size_t i=1; i<n_subcomponents;i++){
                        if(subcomponents[i]->getTimeNext() < localNext){
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
                        if(localNext < timeNext){
                            timeNext = localNext;
                        }
                    }
					#pragma omp barrier
                    //end Step 4

                }//end simulation loop
            }
        }

        // TODO what is the difference between simulate, simulate_serial, and simulate_stacked?
        void simulate_serial(double timeInterval, size_t thread_number = std::thread::hardware_concurrency()) {
        	double timeNext = topCoordinator->getTimeNext();
            double timeFinal = topCoordinator->getTimeLast() + timeInterval;

            //threads created
			#pragma omp parallel default(none) num_threads(thread_number) shared(timeNext, timeFinal, topCoordinator)
            {
                //each thread get its if within the group
                size_t tid = omp_get_thread_num();
                //get list of subcomponents
                auto& subcomponents = topCoordinator->getSubcomponents();
                //get number of subcomponents
                auto n_subcomponents = subcomponents.size();
                //auto& internal_couplings = topCoordinator->getSerialIC();
                //get number of internal couplings
                auto n_internal_couplings = stackedIC.size();
                double localNext;

                while(timeNext < timeFinal) {
                    // Step 1: execute output functions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                    	subcomponents.at(i)->collection(timeNext);
                    }
					#pragma omp barrier
                    //end Step 1

/*
					// Step 2: route messages
					#pragma omp for schedule(static)
					for(size_t i=0; i<n_internal_couplings;i++) {
                    	internal_couplings.at(i).first->parallelPropagate(internal_couplings.at(i).second);
                    }
                    // end Step 2
*/
            		// Step 2: route messages
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_internal_couplings;i++){
                    	for(auto& portFrom: stackedIC[i].second){
                            stackedIC.at(i).first->propagate(portFrom);
                    	}
                    }
					#pragma omp barrier
                    // end Step 2

                    // Step 3: state transitions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                        subcomponents.at(i)->transition(timeNext);
                        subcomponents.at(i)->clear();
                    }
					#pragma omp barrier
                    // end Step 3

                    // Step 4: time for next events
                    localNext = subcomponents[0]->getTimeNext();
					#pragma omp for schedule(static)
                    for(size_t i=1; i<n_subcomponents;i++){
                        if(subcomponents[i]->getTimeNext() < localNext){
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
                        if(localNext < timeNext){
                            timeNext = localNext;
                        }
                    }
					#pragma omp barrier
                    //end Step 4

                }//end simulation loop
            }
        }

        // TODO what is the difference between simulate, simulate_serial, and simulate_stacked?
        void simulate_stacked(double timeInterval, size_t thread_number = std::thread::hardware_concurrency()) {
        	double timeNext = topCoordinator->getTimeNext();
            double timeFinal = topCoordinator->getTimeLast()+timeInterval;

            //threads created
			#pragma omp parallel default(none) num_threads(thread_number) shared(timeNext, timeFinal, topCoordinator)
            {
                //each thread get its if within the group
                size_t tid = omp_get_thread_num();
                //get list of subcomponents
                auto& subcomponents = topCoordinator->getSubcomponents();
                //get number of subcomponents
                auto n_subcomponents = subcomponents.size();
                //auto& internal_couplings = topCoordinator->getSerialIC();
                //get number of internal couplings
                auto n_internal_couplings = stackedIC.size();
                double localNext;

                while(timeNext < timeFinal) {
                    // Step 1: execute output functions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                    	subcomponents.at(i)->collection(timeNext);
                    }
					#pragma omp barrier
                    //end Step 1

/*
					// Step 2: route messages
					#pragma omp for schedule(static)
					for(size_t i=0; i<n_internal_couplings;i++) {
                    	internal_couplings.at(i).first->parallelPropagate(internal_couplings.at(i).second);
                    }
                    // end Step 2
*/
            		// Step 2: route messages
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_internal_couplings;i++){
                    	for(auto& portFrom: stackedIC[i].second){
                            stackedIC.at(i).first->propagate(portFrom);
                    	}
                    }
					#pragma omp barrier
                    // end Step 2

                    // Step 3: state transitions
					#pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcomponents;i++){
                        subcomponents.at(i)->transition(timeNext);
                        subcomponents.at(i)->clear();
                    }
					#pragma omp barrier
                    // end Step 3

                    // Step 4: time for next events
                    localNext = subcomponents[0]->getTimeNext();
					#pragma omp for schedule(static)
                    for(size_t i=1; i<n_subcomponents;i++){
                        if(subcomponents[i]->getTimeNext() < localNext){
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
                        if(localNext < timeNext){
                            timeNext = localNext;
                        }
                    }
					#pragma omp barrier
                    //end Step 4

                }//end simulation loop
            }
        }

        void sequential_simulate(double timeInterval) {  // TODO I think we should remove this method
        	double timeNext = topCoordinator->getTimeNext();
        	double timeFinal = topCoordinator->getTimeLast()+timeInterval;
        	//get list of subcomponents
        	auto& subcomponents = topCoordinator->getSubcomponents();
        	//get number of subcomponents
        	auto n_subcomponents = subcomponents.size();
        	//auto& internal_couplings = topCoordinator->getSerialIC();
        	auto n_internal_couplings = stackedIC.size();

        	while(timeNext < timeFinal) {

        		// Step 1: execute output functions
        		for(size_t i=0; i<n_subcomponents;i++){
        			subcomponents.at(i)->collection(timeNext);
        		}
        		//end Step 1

        		// Step 2: route messages
                for(size_t i=0; i<n_internal_couplings;i++){
                	for(auto& portFrom: stackedIC[i].second){
                        stackedIC.at(i).first->propagate(portFrom);
                	}
                }
                // end Step 2

        		// Step 3: state transitions
        		for(size_t i=0; i<n_subcomponents;i++){
        			subcomponents.at(i)->transition(timeNext);
        			subcomponents.at(i)->clear();
        		}
        		// end Step 3

        		//Step 4
        		timeNext = subcomponents.at(0)->getTimeNext();
        		for(size_t i=1; i<n_subcomponents;i++){
        			if(subcomponents.at(i)->getTimeNext() < timeNext){
        				timeNext = subcomponents.at(i)->getTimeNext();
                    }
        		}
        		// end Step 4

        	}//end simulation loop
        }

    };
}

#endif //CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
