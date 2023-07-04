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
#define SEC_TO_MICRO 1000000.0
#define MILI_TO_MICRO 1000.0

#include "rt_clock.hpp"
#include "../../exception.hpp"
#include <timer_a.h>
#include <timer32.h>
#include <cs.h>
#include <interrupt.h>
#include <rom.h>

static volatile bool expired;
static volatile long numSeconds = 0;

// * TimerA used to measure time between waitUntil calls
// * Timer32 used to measure time for system to wait for next event

const Timer_A_UpModeConfig upConfig =
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    31999,
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR
};

void T32_INT1_IRQHandler(void)
{
    ROM_Timer32_clearInterruptFlag (TIMER32_0_BASE); // reset interrupt flag
    expired = true;
}

void TA3_0_IRQHandler(void)
{
    ROM_Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0); // reset interrupt flag
    numSeconds++;
}

namespace cadmium {
    /**
     * Real-time clock based on the Driver APIs provided by TI for MSP432P401R
     * @tparam T Internal clock type. By default, it uses a double.
     */
    template <typename T = double>
    class TIClock : RealTimeClock {
    private:
    protected:
        T rTimeLast;
        long maxJitter; //! Maximum allowed delay jitter. This parameter is optional.
    public:
        volatile bool interrupted;

        //! The empty constructor does not check the accumulated delay jitter.
        TIClock(): RealTimeClock(){
            expired = false;
            interrupted = false;
            maxJitter = -1;
            rTimeLast = 0.0;
            ROM_CS_setReferenceOscillatorFrequency(CS_REFO_32KHZ);
            ROM_CS_initClockSignal(CS_MCLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
            ROM_Timer32_initModule(TIMER32_0_BASE,TIMER32_PRESCALER_1,TIMER32_32BIT,TIMER32_PERIODIC_MODE);
            ROM_Interrupt_enableSleepOnIsrExit();
            ROM_Interrupt_enableInterrupt(TIMER32_0_INTERRUPT);
            ROM_Timer32_enableInterrupt(TIMER32_0_BASE);
            ROM_CS_initClockSignal(CS_SMCLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
            ROM_Timer_A_configureUpMode(TIMER_A3_BASE,&upConfig);
            ROM_Interrupt_enableInterrupt(INT_TA3_0);
            ROM_Interrupt_enableMaster();
        }

        /**
	     * Use this constructor to select the maximum allowed delay jitter.
	     * @param maxJitter duration of the maximum allowed jitter (long).
	     */
        [[maybe_unused]] explicit TIClock(long maxJitter): TIClock() {
		    maxJitter = maxJitter;
	    }

        /**
         * Starts the real-time clock.
         * @param timeLast initial simulation time.
         */
        void start(double timeLast) override {
            RealTimeClock::start(timeLast);
            rTimeLast = 0.0;
            numSeconds = 0;
            ROM_Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_UP_MODE);
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {
            ROM_Timer_A_stopTimer(TIMER_A3_BASE);
            RealTimeClock::stop(timeLast);
        }

        /**
         * Waits until the next simulation time.
         * @param nextTime next simulation time (in seconds).
         */
        T waitUntil(double timeNext) override {
            ROM_Timer_A_stopTimer(TIMER_A3_BASE);
            long expected = (timeNext-vTimeLast == std::numeric_limits<double>::infinity())?std::numeric_limits<long>::max():((timeNext-vTimeLast-numSeconds)*SEC_TO_MICRO + ROM_Timer_A_getCounterValue(TIMER_A3_BASE)*MILI_TO_MICRO/32);
            long actual = 0;
            expired = false;
            numSeconds = 0;
            ROM_Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_UP_MODE);
            long left = expected;
            while(!interrupted && left > 0){
                expired = false;
                long temp = std::min((long)SEC_TO_MICRO,left);
                ROM_Timer32_setCount(TIMER32_0_BASE,3*temp-1);
                ROM_Timer32_startTimer(TIMER32_0_BASE,true);
                while(!expired && !interrupted) __sleep();
                if(!interrupted) left-=temp;
            }
            expired = false;
            ROM_Timer_A_stopTimer(TIMER_A3_BASE);
            left -= (numSeconds*SEC_TO_MICRO+ROM_Timer_A_getCounterValue(TIMER_A3_BASE)*MILI_TO_MICRO/32);
            actual = std::max(expected-left,0l);
            double duration = actual*SEC_TO_MICRO;
            rTimeLast += duration;
            return RealTimeClock::waitUntil(vTimeLast + duration);
        }

        void update(){
            interrupted = true;
        }
    };
} // namespace cadmium

#endif // CADMIUM_SIMULATION_RT_CLOCK_TI_HPP
