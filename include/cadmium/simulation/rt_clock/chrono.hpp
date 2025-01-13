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

#include "../../modeling/devs/component.hpp" //for the interrupt
#include "../../modeling/devs/coupled.hpp" //for the interrupt

#include "interrupt_handler.hpp"

namespace cadmium {
    /**
     * Real-time clock based on the std::chrono library. It is suitable for Linux, MacOS, and Windows.
     * @tparam T Internal clock type. By default, it uses the std::chrono::steady_clock
     */
    template<typename T = std::chrono::steady_clock, typename Y = uint64_t, typename Z = InterruptHandler<Y>>
    class ChronoClock : RealTimeClock {
     protected:
        std::chrono::time_point<T> rTimeLast; //!< last real system time.
        std::shared_ptr<Coupled> top_model;
        std::shared_ptr<InterruptHandler<Y>> ISR_handle;
        bool IE;
        double startTime;
        std::optional<typename T::duration> maxJitter; //!< Maximum allowed delay jitter. This parameter is optional.

     public:

        //! The empty constructor does not check the accumulated delay jitter.
        ChronoClock() : RealTimeClock(), rTimeLast(T::now()) {
            this->top_model = NULL;
            IE = false;
            startTime = std::chrono::duration<double>(T::now().time_since_epoch()).count();
        }

        [[maybe_unused]] explicit ChronoClock(std::shared_ptr<Coupled> model) : ChronoClock() {
            IE = true;
            this->top_model = model;
            this->ISR_handle = std::make_shared<Z>();
        }

        [[maybe_unused]] explicit ChronoClock(typename T::duration maxJitter) : ChronoClock() {
            this->top_model = NULL;
            IE = false;
            startTime = std::chrono::duration<double>(T::now().time_since_epoch()).count();
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

            cadmium::Component IC("Interrupt Component");
            cadmium::BigPort<Y> out;
            out = IC.addOutBigPort<Y>("out");
            
            while(T::now() < rTimeLast || timeNext == std::numeric_limits<double>::infinity()) {
                if(IE){
                    if (ISR_handle->ISRcb()) {
                        auto data = ISR_handle->decodeISR();

                        auto epoch = T::now().time_since_epoch();
                        double time_now = std::chrono::duration<double>(epoch).count();

                        out->addMessage(data);
                        top_model->getInPort("in")->propagate(out);

                        rTimeLast = T::now();
                        break;
                    }
		    std::this_thread::sleep_for(std::chrono::microseconds(1));
                } else {
                    std::this_thread::yield();
                }
            }

#ifdef DEBUG_DELAY
            std::cout << "[DELAY] " << std::chrono::duration_cast<std::chrono::microseconds>(T::now() - rTimeLast) << std::endl;
#endif
            if (maxJitter.has_value()) {
                auto jitter = T::now() - rTimeLast;
                if (jitter > maxJitter.value()) {
                    throw cadmium::CadmiumRTClockException("delay jitter is too high");
                }
            }
            
            auto epoch = T::now().time_since_epoch();
            double time_now = std::chrono::duration<double>(epoch).count();
            return RealTimeClock::waitUntil(std::min(timeNext, time_now - startTime));
        }
    };
}

#endif // CADMIUM_SIMULATION_RT_CLOCK_CHRONO_HPP
