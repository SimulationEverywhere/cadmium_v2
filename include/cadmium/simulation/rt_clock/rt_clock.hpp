/**
 * Abstract Base Class of a real-time clock.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
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
