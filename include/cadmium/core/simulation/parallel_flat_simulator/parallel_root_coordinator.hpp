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

#ifndef _CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
#define _CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_

#include <memory>
#include <utility>
#include <vector>
#include "parallel_abstract_simulator.hpp"
#include "parallel_simulator.hpp"
#include "parallel_flat_coordinator.hpp"
#include "../modeling/atomic.hpp"
#include "../modeling/coupled.hpp"

namespace cadmium {
    class ParallelRootCoordinator {
     private:
        flat_coordinator top_coordinator;

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

            //create parallel region//
            #pragma omp parallel num_threads(_thread_number) shared(timeNext, timeFinal, top_coordinator) //proc_bind(close)
            {
                #pragma omp critical
                {
                    cadmium:dynamic::hpc_engine::pin_thread_to_core(tid);
                }

                std::vector<std::shared_ptr<AbstractSimulator>> subcoordinators = top_coordinator.get_subcoordinators();
                std::vector<coupling> internal_couplings = top_coordinator.get_internal_couplings();

                size_t n_subcoordinators = subcoordinators.size();
                size_t n_internal_couplings = internal_couplings.size();

                double local_next;

                //each thread gets its id//
                int tid = omp_get_thread_num();

                while(timeNext < timeFinal) {

                    #pragma omp master
                	{
                        timeLast = timeNext;
                    }

                    // Step 1: execute output functions
                	//top_coordinator.execute_output_functions(timeNext);
                    #pragma omp for schedule(static)
                    for(size_t i=0; i<n_subcoordinators;i++){
                	    subcoordinators.at(i)->execute_output_function(timeNext);
                    }
                    //end step 1

                    // Step 2: route messages
                	//top_coordinator.route_messages();
                    #pragma omp for schedule(static)
                    for(size_t i=0; i<n_internal_couplings;i++){
                        std::get<1>(internal_couplings.at(i))->propagate(std::get<0>(internal_couplings.at(i)));
                    }
                	// end step 2

                    // Step 3:
                    //timeNext = top_coordinator.execute_state_transitions(timeNext);
                    #pragma omp for schedule(static)
                    for(){
                        subcoordinators.at(i)->execute_output_function(timeNext);
                    }

                    //

                    top_coordinator.clear();
                }
            }
            timeLast = timeFinal;
        }
    };
}

#endif //_CADMIUM_CORE_SIMULATION_COORDINATOR_HPP_
