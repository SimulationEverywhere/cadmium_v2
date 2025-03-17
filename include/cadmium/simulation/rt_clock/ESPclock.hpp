/**
 * Real-time clock based for the ESP32.
 * Copyright (C) 2023  Sasisekhar Mangalam Govind
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

#ifndef CADMIUM_SIMULATION_RT_CLOCK_ESP_HPP
#define CADMIUM_SIMULATION_RT_CLOCK_ESP_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "rt_clock.hpp"
#include "../../exception.hpp"
#include "interrupt_handler.hpp"
#include <variant>

namespace cadmium {
    /**
     * Real-time clock based on the std::chrono library. It is suitable for Linux, MacOS, and Windows.
     * @tparam T Internal clock type. By default, it uses the std::chrono::steady_clock
     */
    template<typename T = double, typename variantType = std::variant<int64_t>>
    class ESPclock : RealTimeClock {
    private:
        gptimer_handle_t executionTimer;
        double rTimeLast;
        std::shared_ptr<Coupled> top_model;
        std::shared_ptr<InterruptHandler<variantType>> ISR_handle;
        bool IE;

        void initTimer() {
            gptimer_config_t timer_config1 = {
                .clk_src = GPTIMER_CLK_SRC_DEFAULT,
                .direction = GPTIMER_COUNT_UP,
                .resolution_hz = 40 * 1000 * 1000, // 10MHz, 1 tick=100ns
                .intr_priority = 0,
                .flags = {
                    .intr_shared = true,
                    .allow_pd = false,

                    .backup_before_sleep = false //*!< @deprecated, but quiets warnings
                }

            };
            ESP_ERROR_CHECK(gptimer_new_timer(&timer_config1, &executionTimer));

            ESP_ERROR_CHECK(gptimer_enable(executionTimer));
            gptimer_set_raw_count(executionTimer, 0);
            gptimer_start(executionTimer);
        }

     public:

        //! The empty constructor does not check the accumulated delay jitter.
        ESPclock() : RealTimeClock() {
            initTimer();
            this->top_model = NULL;
            IE = false;
        }

        [[maybe_unused]] ESPclock(std::shared_ptr<Coupled> model, std::shared_ptr<InterruptHandler<variantType>> handler = nullptr) : RealTimeClock() {
            initTimer();

            IE = (handler != nullptr);          //!< Enable interrupts if the handler is provided.

            this->top_model = model;
            this->ISR_handle = handler;          //!< Set the interrupt handler.
        }

        /**
         * Starts the real-time clock.
         * @param timeLast initial simulation time.
         */
        void start(double timeLast) override {
            RealTimeClock::start(timeLast);
            uint64_t count = 0;
            uint32_t res = 0;
            gptimer_get_resolution(executionTimer, &res);
            gptimer_get_raw_count(executionTimer, &count);
            rTimeLast = (double)count / (double)res;
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {
            uint64_t count = 0;
            uint32_t res = 0;
            gptimer_get_resolution(executionTimer, &res);
            gptimer_get_raw_count(executionTimer, &count);
            rTimeLast = (double)count / (double)res;
            RealTimeClock::stop(timeLast);
        }

        /**
         * Waits until the next simulation time or until an external event happens.
         *
         * @param timeNext next simulation time (in seconds) for an internal transition.
         * @return next simulation time (in seconds). Return value must be less than or equal to timeNext.
         * */
        double waitUntil(double timeNext) override {
            auto duration = timeNext - vTimeLast;
            rTimeLast += duration;

            uint64_t count = 0;
            uint32_t res = 0;

            gptimer_get_resolution(executionTimer, &res);
            gptimer_get_raw_count(executionTimer, &count);
            double timeNow = (double)count / (double)res;

            while(timeNow < rTimeLast) {
                gptimer_get_resolution(executionTimer, &res);
                gptimer_get_raw_count(executionTimer, &count);
                timeNow = (double)count / (double)res;

                if(IE){
                    if (ISR_handle->ISRcb()) {
                        auto data = ISR_handle->decodeISR();
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
                        rTimeLast = timeNow;
                        break;
                    }
                }
            
            }


#ifdef DEBUG_DELAY
            ESP_LOGI("ESPclock", "[DELAY] %f us", (timeNow - rTimeLast) * 1000 * 1000);
#endif
            return RealTimeClock::waitUntil(std::min(timeNext, timeNow));
        }
    };
}

#endif // CADMIUM_SIMULATION_RT_ESP_CLOCK_HPP
