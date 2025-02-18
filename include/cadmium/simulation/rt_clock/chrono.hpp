/**
 * Real-time clock based on the chrono standard library.
 * Copyright (C) 2023  Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 * 2025 Sasisekhar Mangalam Govind
 * ARSLab - Carleton University
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
#include <variant>
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
    template<typename T = std::chrono::steady_clock, typename variantType = std::variant<int64_t>>
    class ChronoClock : RealTimeClock {
    protected:
        std::chrono::time_point<T> rTimeLast;
        std::shared_ptr<Coupled> top_model;
        std::shared_ptr<InterruptHandler<variantType>> ISR_handle;
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

        //! Constructor accepting both a top model and an interrupt handler.
        //! This constructor initializes the real-time clock with a model and a handler for asynchronous inputs.
        //! If no handler is provided, it defaults to nullptr, and interrupts will be disabled.
        //! @param model Pointer to the coupled top model.
        //! @param handler Shared pointer to the interrupt handler. Defaults to nullptr.
        [[maybe_unused]] explicit ChronoClock(std::shared_ptr<Coupled> model, std::shared_ptr<InterruptHandler<variantType>> handler = nullptr)
            : ChronoClock() {
            IE = (handler != nullptr);          //!< Enable interrupts if the handler is provided.

            this->top_model = model;
            this->ISR_handle = handler;          //!< Set the interrupt handler.
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
            
            while(T::now() < rTimeLast || timeNext == std::numeric_limits<double>::infinity()) {
                if(IE){
                    if (ISR_handle->ISRcb()) {
                        auto data = ISR_handle->decodeISR();

                        auto epoch = T::now().time_since_epoch();
                        double time_now = std::chrono::duration<double>(epoch).count();

                        // Use std::visit to handle the variant type
                        std::visit([&](auto&& value) {
                            using ActualType = std::decay_t<decltype(value)>;

                            bool isBigPort = std::dynamic_pointer_cast<_BigPort<ActualType>>(top_model->getInPort(data.second)) != nullptr;

                            if(isBigPort){
                                cadmium::BigPort<ActualType> out;
                                cadmium::Component IC("Interrupt Component");
                                out = IC.addOutBigPort<ActualType>("out");
                                out->addMessage(value);

                                if (top_model->getInPort(data.second)->compatible(out)){
                                    top_model->getInPort(data.second)->propagate(out);
                                } else {
                                    std::cerr << "[Interrupt Component] Incompatible ports!!" << std::endl;
                                }
                            } else {
                                cadmium::Port<ActualType> out;
                                cadmium::Component IC("Interrupt Component");
                                out = IC.addOutPort<ActualType>("out");
                                out->addMessage(value);

                                if (top_model->getInPort(data.second)->compatible(out)){
                                    top_model->getInPort(data.second)->propagate(out);
                                } else {
                                    std::cerr << "[Interrupt Component] Incompatible ports!!" << std::endl;
                                }
                            }

                        }, data.first);


                        rTimeLast = T::now();
                        break;
                    }

		            std::this_thread::sleep_for(std::chrono::microseconds(1)); //<! This reduces CPU consumption when waiting
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
