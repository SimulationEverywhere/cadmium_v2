/**
 * Real-time clock based on the chrono standard library.
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

#ifndef CADMIUM_SIMULATION_RT_CLOCK_CHRONO_HPP
#define CADMIUM_SIMULATION_RT_CLOCK_CHRONO_HPP

#include <iostream>
#include <chrono>
#include <optional>
#include <thread>
#include "rt_clock.hpp"
#include "../../exception.hpp"

namespace cadmium {
    /**
     * Real-time clock based on the std::chrono library. It is suitable for Linux, MacOS, and Windows.
     * @tparam T Internal clock type. By default, it uses the std::chrono::steady_clock
     */
    template<typename T = std::chrono::steady_clock>
    class ChronoClock : RealTimeClock {
     protected:
        std::chrono::time_point<T> rTimeLast; //!< last real system time.
        std::optional<typename T::duration> maxJitter; //!< Maximum allowed delay jitter. This parameter is optional.
     public:

        //! The empty constructor does not check the accumulated delay jitter.
        ChronoClock() : RealTimeClock(), rTimeLast(T::now()), maxJitter() {}

        /**
         * Use this constructor to select the maximum allowed delay jitter.
         * @param maxJitter duration of the maximum allowed jitter.
         */
        [[maybe_unused]] explicit ChronoClock(typename T::duration maxJitter) : ChronoClock() {
            this->maxJitter.emplace(maxJitter);
        }

        /**
         * Starts the real-time clock.
         * @param timeLast initial simulation time.
         */
        void start(double timeLast) override {
            RealTimeClock::start(timeLast);
            rTimeLast = T::now();
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {
            rTimeLast = T::now();
            RealTimeClock::stop(timeLast);
        }

        /**
         * Waits until the next simulation time or until an external event happens.
         *
         * @param nextTime next simulation time (in seconds) for an internal transition.
         * @return next simulation time (in seconds). Return value must be less than or equal to nextTime.
         * */
        double waitUntil(double timeNext) override {
            auto duration =
                std::chrono::duration_cast<typename T::duration>(std::chrono::duration<double>(timeNext - vTimeLast));
            rTimeLast += duration;
            std::this_thread::sleep_until(rTimeLast);
            if (maxJitter.has_value()) {
                auto jitter = T::now() - rTimeLast;
                if (jitter > maxJitter.value()) {
                    throw cadmium::CadmiumRTClockException("delay jitter is too high");
                }
            }
            return RealTimeClock::waitUntil(timeNext);
        }
    };
}

#endif // CADMIUM_SIMULATION_RT_CLOCK_CHRONO_HPP
