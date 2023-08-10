/**
 * Abstract Base Class of a real-time clock.
 * Copyright (C) 2023  Román Cárdenas Rodríguez
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

#ifndef CADMIUM_SIMULATION_RT_CLOCK_RT_CLOCK_HPP
#define CADMIUM_SIMULATION_RT_CLOCK_RT_CLOCK_HPP

namespace cadmium {
    //! Abstract base class of a real-time clock.
    class RealTimeClock {
     protected:
        double vTimeLast;  //!< Last virtual time (i.e., the clock time in the simulation).
     public:

        RealTimeClock() : vTimeLast() {}

        virtual ~RealTimeClock() = default;

        /**
         * Virtual method for starting the real-time clock.
         * @param timeLast initial virtual time (in seconds).
         */
        virtual void start(double timeLast) {
            vTimeLast = timeLast;
        };

        /**
         * Virtual method for stopping the real-time clock.
         * @param timeLast final virtual time (in seconds).
         */
        virtual void stop(double timeLast) {
            vTimeLast = timeLast;
        }

        /**
         * Waits until the next simulation time or until an external event happens.
         * In this abstract implementation, it does nothing. Thus, it always return timeNext.
         *
         * @param nextTime next simulation time (in seconds).
         * @return next simulation time (in seconds). Return value must be less than or equal to nextTime.
         */
        virtual double waitUntil(double timeNext) {
            vTimeLast = timeNext;
            return vTimeLast;
        }
    };
}

#endif // CADMIUM_SIMULATION_RT_CLOCK_RT_CLOCK_HPP
