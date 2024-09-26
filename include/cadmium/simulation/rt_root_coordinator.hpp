/**
 * Root coordinator for real-time sequential simulation.
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

#ifndef CADMIUM_SIMULATION_RT_ROOT_COORDINATOR_HPP_
#define CADMIUM_SIMULATION_RT_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "root_coordinator.hpp"
#include "core/coordinator.hpp"
#include "rt_clock/rt_clock.hpp"

namespace cadmium {
    /**
     * Real-time root coordinator.
     * @tparam T type of the real-time clock.
     */
    template <typename T>
    class RealTimeRootCoordinator: public RootCoordinator {
        static_assert(std::is_base_of<RealTimeClock, T>::value, "T must inherit cadmium::RealTimeClock");
     protected:
        T clock;  //!< Real-time clock.

        /**
         * The only difference with the sequential simulation is that we first wait until timeNext.
         * @param timeNext next simulation time (in seconds).
         */
        void simulationAdvance(double timeNext) override {
            double t = clock.waitUntil(timeNext);
            // double t = timeNext;
            RootCoordinator::simulationAdvance(t);
        }

     public:
        RealTimeRootCoordinator(std::shared_ptr<Coupled> model, double time, T clock):
            RootCoordinator(model, time), clock(clock) {}

        RealTimeRootCoordinator(std::shared_ptr<Coupled> model, T clock): RealTimeRootCoordinator(model, 0, clock) {}

        //! Starts the root coordinator and the real-time clock.
        void start() override {
            RootCoordinator::start();
            clock.start(topCoordinator->getTimeLast());
        }

        //! Stops the root coordinator and the real-time clock.
        void stop() override {
            clock.stop(topCoordinator->getTimeLast());
            RootCoordinator::stop();
        }
    };
}

#endif // CADMIUM_SIMULATION_RT_ROOT_COORDINATOR_HPP_
