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

#ifndef CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
#define CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <utility>
#include <vector>
#include "root_coordinator.hpp"
#include "../logger/logger.hpp"
#include <omp.h>
#include <thread>

namespace cadmium {
	//! Parallel Root coordinator class.
    class ParallelRootCoordinator: public RootCoordinator {
     public:
        ParallelRootCoordinator(std::shared_ptr<Coupled> model, double time):
        	RootCoordinator(model, time) {}
		explicit ParallelRootCoordinator(std::shared_ptr<Coupled> model): ParallelRootCoordinator(std::move(model), 0) {}
/*
		void simulate(long nIterations) {
			double timeNext = topCoordinator->getTimeNext();
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
				simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }
*/
		void simulate(double timeInterval, size_t thread_number = std::thread::hardware_concurrency()) {
			double timeNext = getTopCoordinator()->getTimeNext();
			double timeFinal = getTopCoordinator()->getTimeLast()+timeInterval;

			//threads created
			#pragma omp parallel num_threads(thread_number) shared(timeNext, timeFinal) //proc_bind(close)
			{
				//each thread pins itself to a core
				#pragma omp critical
				{
					//pin_thread_to_core(tid);
				}

				//get list of subcomponents
                auto subcomponents = getTopCoordinator()->getSubcomponents();

                //get number of subcomponents
                auto n_subcomponents = subcomponents.size();

                //get list of internal_couplings
                auto coupled = std::dynamic_pointer_cast<Coupled>(getTopCoordinator()->getComponent());
                auto internal_couplings = coupled->getICs();

                //get number of internal couplings
                size_t n_internal_couplings = internal_couplings.size();

                double localNext;

				//each thread get its if within the group
				size_t tid = omp_get_thread_num();

				while(timeNext < timeFinal) {
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
                        std::get<1>(internal_couplings.at(i))->propagate(std::get<0>(internal_couplings.at(i)));
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
    };

}

#endif //CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
