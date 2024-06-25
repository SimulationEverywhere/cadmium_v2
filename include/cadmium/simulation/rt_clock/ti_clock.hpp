/**
 * Real-time clock for the MSP432P401R board
 * Copyright (C) 2023  Srijan Gupta & James Grieder
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

#ifndef CADMIUM_SIMULATION_RT_CLOCK_TI_HPP
#define CADMIUM_SIMULATION_RT_CLOCK_TI_HPP
#define MILI_TO_MICRO 1000.0

#include <limits>
#include "rt_clock.hpp"
#include "../../exception.hpp"
#include <cs.h>
#include <timer32.h>
#include <interrupt.h>
#include <timer_a.h>
#include <rom.h>

volatile bool expired = false;
volatile long numSeconds = 0;

extern "C" {
const Timer_A_UpModeConfig config = {
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    127,
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR
};

void T32_INT1_IRQHandler(void){
    ROM_Timer32_clearInterruptFlag(TIMER32_0_BASE); // reset interrupt flag
    numSeconds++;
}

void TA3_0_IRQHandler(void){
    ROM_Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
    expired = true;
}
}

namespace cadmium {
    /**
     * Real-time clock based on the Timer32 and TimerA APIs provided by TI for MSP432P401R
     */
    class TIClock : RealTimeClock {
    private:
    protected:
        double rTimeLast;
    public:
        volatile bool interrupted;

        /** This constructor configures the timer
         */ 
        TIClock(): RealTimeClock(){
            interrupted = false;
            ROM_CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
            ROM_CS_initClockSignal(CS_SMCLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
            ROM_CS_initClockSignal(CS_MCLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
            ROM_Timer_A_configureUpMode(TIMER_A3_BASE,&config);
            ROM_Timer32_initModule(TIMER32_0_BASE,TIMER32_PRESCALER_1,TIMER32_32BIT,TIMER32_PERIODIC_MODE);
            ROM_Interrupt_enableInterrupt(INT_TA3_0);
            ROM_Interrupt_enableInterrupt(TIMER32_0_INTERRUPT);
            ROM_Timer32_enableInterrupt(TIMER32_0_BASE);
            ROM_Interrupt_enableMaster();
        }

        /**
         * Starts the real-time clock.
         * @param timeLast initial simulation time.
         */
        void start(double timeLast) override {
            RealTimeClock::start(timeLast);
            rTimeLast = 0.0;
            numSeconds = 0;
            ROM_Timer32_setCount(TIMER32_0_BASE,128000);
            ROM_Timer32_startTimer(TIMER32_0_BASE,false);
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {
            ROM_Timer32_haltTimer(TIMER32_0_BASE);
            rTimeLast = timeLast;
            RealTimeClock::stop(timeLast);
        }

        /**
         * Waits until the next simulation time.
         * @param nextTime next simulation time (in seconds).
         */
        double waitUntil(double timeNext) override {
            // TODO Use timer32 to measure time required for computations and timer_a for delay.
            ROM_Timer32_haltTimer(TIMER32_0_BASE);
            long internal = numSeconds*MILI_TO_MICRO + (128064-ROM_Timer32_getValue(TIMER32_0_BASE))/128;
            long expected = std::max((timeNext-vTimeLast)*MILI_TO_MICRO + 0.5 - internal, 1.0);
            long left = expected;
            expired = false;
            ROM_Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_UP_MODE);
            while(!interrupted && left){
                expired = false;
                while(!expired && !interrupted){}
                if(!interrupted) left--;
            }
            ROM_Timer_A_stopTimer(TIMER_A3_BASE);
            double duration = (expected+internal-left)/MILI_TO_MICRO;
            rTimeLast += duration;
            numSeconds = 0;
            ROM_Timer32_setCount(TIMER32_0_BASE,128000);
            ROM_Timer32_startTimer(TIMER32_0_BASE,false);
            return RealTimeClock::waitUntil(vTimeLast+duration);
        }

        void update(){
            interrupted = true;
        }
    };
} // namespace cadmium

#endif // CADMIUM_SIMULATION_RT_CLOCK_TI_HPP
