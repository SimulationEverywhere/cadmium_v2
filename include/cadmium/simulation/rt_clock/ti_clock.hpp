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
// #define SEC_TO_MICRO 1000000.0
#define MILI_TO_MICRO 1000.0

#include <limits>
#include "rt_clock.hpp"
#include "../../exception.hpp"
#include <cs.h>
#include <timer32.h>
#include <interrupt.h>

// static volatile bool expired;
static volatile long numSeconds = 0;

// * Timer32 used to measure time in between last setting of vTimeLast and next call of waitUntil.

extern "C" { // Required to prevent name-mangling
    void T32_INT1_IRQHandler(void){
        Timer32_clearInterruptFlag (TIMER32_0_BASE); // reset interrupt flag
        numSeconds++;
    }

    void delay(long count){
    __asm__(
        "subs    r0, #1\n"
        "bne     delay\n"
        "bx      lr\n"
    );
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
            CS_setExternalClockSourceFrequency(32000,CS_48MHZ);
            CS_startHFXT(false);
            Timer32_initModule(TIMER32_0_BASE,TIMER32_PRESCALER_16,TIMER32_32BIT,TIMER32_PERIODIC_MODE);
            Interrupt_enableInterrupt(TIMER32_0_INTERRUPT);
            Timer32_enableInterrupt(TIMER32_0_BASE);
            Interrupt_enableMaster();
        }

        /**
         * Starts the real-time clock.
         * @param timeLast initial simulation time.
         */
        void start(double timeLast) override {
            RealTimeClock::start(timeLast);
            rTimeLast = 0.0;
            numSeconds = 0;
            Timer32_setCount(TIMER32_0_BASE,CS_3MHZ);
            Timer32_startTimer(TIMER32_0_BASE,false);
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {
            Timer32_haltTimer(TIMER32_0_BASE);
            RealTimeClock::stop(timeLast);
        }

        /**
         * Waits until the next simulation time.
         * @param nextTime next simulation time (in seconds).
         */
        double waitUntil(double timeNext) override {
            // ROM_Timer_A_stopTimer(TIMER_A3_BASE);
            // numSeconds = 0;
            // long expected = (timeNext-vTimeLast == std::numeric_limits<double>::infinity())?std::numeric_limits<long>::max():((timeNext-vTimeLast-numSeconds)*SEC_TO_MICRO + ROM_Timer_A_getCounterValue(TIMER_A3_BASE)*MILI_TO_MICRO/32);
            // long actual = 0;
            // expired = false;
            // numSeconds = 0;
            // ROM_Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_UP_MODE);
            // long left = expected;
            // while(!interrupted && left > 0){
            //     expired = false;
            //     long temp = std::min((long)SEC_TO_MICRO,left);
            //     ROM_Timer32_setCount(TIMER32_0_BASE,temp*32/MILI_TO_MICRO);
            //     ROM_Timer32_startTimer(TIMER32_0_BASE,true);
            //     while(!expired && !interrupted) __sleep();
            //     if(!interrupted) left-=temp;
            // }
            // expired = false;
            // ROM_Timer_A_stopTimer(TIMER_A3_BASE);
            // left -= (numSeconds*SEC_TO_MICRO+ROM_Timer_A_getCounterValue(TIMER_A3_BASE)*MILI_TO_MICRO/32);
            // actual = std::max(expected-left,0l);
            // double duration = actual/SEC_TO_MICRO;
            // rTimeLast += duration;
            // return RealTimeClock::waitUntil(vTimeLast + duration);
            Timer32_haltTimer(TIMER32_0_BASE);
            long expected = (timeNext-vTimeLast-numSeconds)*MILI_TO_MICRO - (CS_3MHZ-Timer32_getValue(TIMER32_0_BASE))/(3*MILI_TO_MICRO); // ! Using milliseconds
            long left = expected;
            while(!interrupted && left){
                delay(8000); // ? Is 8000 the correct value 
                left--;
            }
            numSeconds = 0;
            Timer32_setCount(TIMER32_0_BASE,CS_3MHZ);
            double duration = std::max(expected-left,0l)/MILI_TO_MICRO;
            rTimeLast += duration;
            Timer32_startTimer(TIMER32_0_BASE,false);
            return RealTimeClock::waitUntil(vTimeLast+duration);
        }

        void update(){
            interrupted = true;
        }
    };
} // namespace cadmium

#endif // CADMIUM_SIMULATION_RT_CLOCK_TI_HPP
