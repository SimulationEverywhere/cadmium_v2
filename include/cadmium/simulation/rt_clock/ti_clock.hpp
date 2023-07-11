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
#include <rom.h>

static volatile bool expired;
// static volatile long numSeconds = 0;

// * Timer32 used to measure time in between last setting of vTimeLast and next call of waitUntil.

extern "C" { // Required to prevent name-mangling
    void T32_INT1_IRQHandler(void){
        ROM_Timer32_clearInterruptFlag(TIMER32_0_BASE); // reset interrupt flag
        expired = true;
    }
}

namespace cadmium {
    /**
     * Real-time clock based on the Driver APIs provided by TI for MSP432P401R
     */
    class TIClock : RealTimeClock {
    private:
    protected:
        double rTimeLast;
    public:
        volatile bool interrupted;

        /** This constructor configures the timer
         * Uses inbuilt Timer32 API
         */ 
        TIClock(): RealTimeClock(){
            interrupted = false;
            rTimeLast = 0.0;
            ROM_CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
            ROM_CS_initClockSignal(CS_MCLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
            ROM_Timer32_initModule(TIMER32_0_BASE,TIMER32_PRESCALER_1,TIMER32_16BIT,TIMER32_PERIODIC_MODE);
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
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {
            rTimeLast = timeLast;
            RealTimeClock::stop(timeLast);
        }

        /**
         * Waits until the next simulation time.
         * @param nextTime next simulation time (in seconds).
         */
        double waitUntil(double timeNext) override {
            long expected = std::max((timeNext-vTimeLast)*MILI_TO_MICRO + 0.5, 1.0);
            long left = expected;
            while(!interrupted && left){
                expired = false;
                ROM_Timer32_setCount(TIMER32_0_BASE,128);
                ROM_Timer32_startTimer(TIMER32_0_BASE,true);
                while(!expired && !interrupted){}
                if(!interrupted) left--;
            }
            double duration = (expected-left)/MILI_TO_MICRO;
            rTimeLast += duration;
            return RealTimeClock::waitUntil(vTimeLast+duration);
        }

        void update(){
            interrupted = true;
        }
    };
} // namespace cadmium

#endif // CADMIUM_SIMULATION_RT_CLOCK_TI_HPP
