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

		void simulate(long nIterations) {
			double timeNext = topCoordinator->getTimeNext();
            while (nIterations-- > 0 && timeNext < std::numeric_limits<double>::infinity()) {
				simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }

		void simulate(double timeInterval, size_t thread_number = std::thread::hardware_concurrency()) {
			double timeNext = topCoordinator->getTimeNext();
			double timeFinal = topCoordinator->getTimeLast()+timeInterval;
/*
			#pragma omp parallel num_threads(thread_number) shared(timeNext, timeFinal, topCoordinator) //proc_bind(close)
			{
				size_t tid = omp_get_thread_num();

				#pragma omp critical
				{
					//pin_thread_to_core(tid);
				}


				while(timeNext < timeFinal) {
					simulationAdvance(timeNext);
					timeNext = topCoordinator->getTimeNext();
				}

			}
*/
        }
    };

}

#endif //CADMIUM_CORE_SIMULATION_PARALLEL_ROOT_COORDINATOR_HPP_
