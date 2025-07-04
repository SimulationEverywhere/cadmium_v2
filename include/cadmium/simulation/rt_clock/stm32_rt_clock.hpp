#ifndef CADMIUM_SIMULATION_RT_CLOCK_STM32_HPP
#define CADMIUM_SIMULATION_RT_CLOCK_STM32_HPP

#include "rt_clock.hpp"
#include "../../exception.hpp"
#include "interrupt_handler.hpp"
#include "stm32h7xx_hal.h"  
#include <variant>
#include <memory>
#include <cmath>

#ifdef __cplusplus
extern "C" {
#endif
extern TIM_HandleTypeDef htim2;
#ifdef __cplusplus
}
#endif


namespace cadmium {

template<typename T = double, typename variantType = std::variant<int64_t>>
class STM32Clock : public RealTimeClock {
private:
    double rTimeLast;
    std::shared_ptr<Coupled> top_model;
    std::shared_ptr<InterruptHandler<variantType>> ISR_handle;
    bool IE;

    uint32_t getTimerCount() const {
        return __HAL_TIM_GET_COUNTER(&htim2);
    }

    uint32_t getTimerFreqHz() const {
        // Le timer compte à cette fréquence (ex: 1 MHz → 1 tick = 1 µs)
        return HAL_RCC_GetPCLK1Freq() / (htim2.Init.Prescaler + 1);
    }

public:
    STM32Clock() : RealTimeClock() {
        rTimeLast = 0;
        IE = false;
    }

    STM32Clock(std::shared_ptr<Coupled> model, std::shared_ptr<InterruptHandler<variantType>> handler = nullptr)
        : RealTimeClock(), top_model(model), ISR_handle(handler) {
        IE = (handler != nullptr);
    }

    void start(double timeLast) override {
        RealTimeClock::start(timeLast);
        HAL_TIM_Base_Start(&htim2);
        rTimeLast = (double)getTimerCount() / (double)getTimerFreqHz();
    }

    void stop(double timeLast) override {
        rTimeLast = (double)getTimerCount() / (double)getTimerFreqHz();
        HAL_TIM_Base_Stop(&htim2);
        RealTimeClock::stop(timeLast);
    }

    double waitUntil(double timeNext) override {
        auto duration = timeNext - vTimeLast;
        rTimeLast += duration;

        double timeNow = (double)getTimerCount() / (double)getTimerFreqHz();

        while (timeNow < rTimeLast) {
            timeNow = (double)getTimerCount() / (double)getTimerFreqHz();

            if (IE && ISR_handle->ISRcb()) {
                auto data = ISR_handle->decodeISR();

                std::visit([&](auto&& value) {
                    using ActualType = std::decay_t<decltype(value)>;
                    bool isBigPort = std::dynamic_pointer_cast<_BigPort<ActualType>>(top_model->getInPort(data.second)) != nullptr;

                    if (isBigPort) {
                        cadmium::BigPort<ActualType> out;
                        cadmium::Component IC("Interrupt Component");
                        out = IC.addOutBigPort<ActualType>("out");
                        out->addMessage(value);
                        if (top_model->getInPort(data.second)->compatible(out)) {
                            top_model->getInPort(data.second)->propagate(out);
                        } else {
                            std::cerr << "[Interrupt Component] Incompatible ports!!" << std::endl;
                        }
                    } else {
                        cadmium::Port<ActualType> out;
                        cadmium::Component IC("Interrupt Component");
                        out = IC.addOutPort<ActualType>("out");
                        out->addMessage(value);
                        if (top_model->getInPort(data.second)->compatible(out)) {
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

#ifdef DEBUG_DELAY
        printf("[STM32Clock] [DELAY] %f us\n", (timeNow - rTimeLast) * 1e6);
#endif
        return RealTimeClock::waitUntil(std::min(timeNext, timeNow));
    }
};

} // namespace cadmium

#endif // CADMIUM_SIMULATION_RT_CLOCK_STM32_HPP
