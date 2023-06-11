/**
 * Real-time clock for Mbed-OS version 5
 * Copyright (C) 2023  Ezequiel Pecker Marcosig
 * SEDLab - University of Buenos Aires
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

#ifndef CADMIUM_SIMULATION_RT_CLOCK_MBED_HPP
#define CADMIUM_SIMULATION_RT_CLOCK_MBED_HPP

// #include <iostream>
#include <chrono>
#include <optional>
#include <thread>
#include "rt_clock.hpp"
#include "../../exception.hpp"
#include <mbed.h>

static long MIN_TO_MICRO   = (1000*1000*60);
static long SEC_TO_MICRO   = (1000*1000);
static long MILI_TO_MICRO  = (1000);

namespace cadmium {
    /**
     * Real-time clock based on the Driver APIs provided by MBED-OS version 5: https://os.mbed.com/docs/mbed-os/v5.15/apis/drivers.html.
     * @tparam T Internal clock type. By default, it uses a double.
     */
    template <typename T = double>
    class MBEDClock: RealTimeClock {
     private:
	Timer executionTimer; //!< used in the waitUntil() to measure the elapsed time between consecutive calls from the Root Coordinator and the time elapsed between an event and an external interruption
	Timeout _timeout;
	bool expired;

        //! schedulerSlip = 0 if the next event (actualDelay) is in the future, i.e. we are ahead of schedule
        //! schedulerSlip = <how_long_behind_schedule_we_are> if actualDelay is negative, i.e. we are behind schedule.
        //! This is then added to the next actualDelay and updated until we surpass the tolerance or recover from the slip.
        long schedulerSlip = 0;

	// double e; // elapsed time until interrupt

        /**
         * Returns a long with the time in microseconds (ignore anything below 1 microsecond).
         * @param Time value in seconds.
         */
        long secondsToMicros(const double t) const {
        	return t * SEC_TO_MICRO;
        }

        /**
         * Returns a double with the time in seconds.
         * @param Time in microseconds.
         */
        double microsToSeconds(long us) const {
        	return us / SEC_TO_MICRO;
        }

        /**
         * Sleeps during specified time in microseconds. 
         * @param timeLeft time to sleep (in microseconds).
         */
        long setTimeout(long delayMicroSec) {
        	expired = false;
          	long timeLeft = delayMicroSec;

		executionTimer.reset();

        	//! Handle waits of over ~35 minutes as timer overflows
        	while (!interrupted && (timeLeft > INT_MAX)) {
        	 	this->expired = false;
        	 	this->_timeout.attach_us(callback(this, &MBEDClock::timeoutExpired), INT_MAX);

        	 	while (!expired && !interrupted) {
				sleep();
			}

        	 	if(!interrupted){
        	 		timeLeft -= INT_MAX;
        	 	}
        	}

        	//! Handle waits of under INT_MAX microseconds
        	if(!interrupted && timeLeft > 0) {
        		this->_timeout.attach_us(callback(this, &MBEDClock::timeoutExpired), timeLeft); //!< sets callback to change expired after timeLeft time
        		while (!expired && !interrupted) {
				sleep(); //!< remains here until expired is set true by the _timeout() callback
			}
        	}

		//! if timeLeft < 0 there is no wait

        	executionTimer.stop();
        	expired = false;

		//! in case the sleep() is interrupted by an external interrupt return the remaining time, otherwise return 0
        	if(interrupted) {
        	 	timeLeft -= executionTimer.read_us();
        	 	if(delayMicroSec < timeLeft ) {
				return 0;
			}
        	 	// hal_critical_section_enter();
        	 	// interrupted = false;
        	 	return delayMicroSec - timeLeft;
        	}

        	return 0;
        }

     protected:
        T rTimeLast; //!< last real system time.
        long  maxJitter; //!< Maximum allowed delay jitter. This parameter is optional.

     public:
        volatile bool interrupted; //!< used for external interrupts

        //! The empty constructor does not check the accumulated delay jitter.
        MBEDClock(): RealTimeClock(), executionTimer(), _timeout() {
		rTimeLast = 0.0;
		maxJitter = -1;
		expired = false;
		interrupted = false;
	}

	/**
	 * Use this constructor to select the maximum allowed delay jitter.
	 * @param maxJitter duration of the maximum allowed jitter (long).
	 */
	[[maybe_unused]] explicit MBEDClock(long maxJitter): MBEDClock() {
		maxJitter = maxJitter;
	}


        /**
         * Starts the real-time clock.
         * @param timeLast initial simulation time.
         */
        void start(double timeLast) override {
            RealTimeClock::start(timeLast);
	    rTimeLast = 0.0;
	    executionTimer.reset();
            executionTimer.start();
        }

        /**
         * Stops the real-time clock.
         * @param timeLast last simulation time.
         */
        void stop(double timeLast) override {
            // rTimeLast = executionTimer.read_us();
	    executionTimer.stop();
            RealTimeClock::stop(timeLast);
        }

        /**
         * Waits until the next simulation time.
         * @param nextTime next simulation time (in seconds).
         */
        void waitUntil(double timeNext) override {
            auto duration = timeNext - vTimeLast;
	    long actualDelay;

	    //! if duration is negative then halt and print error over UART
	    if(duration < 0) {
            	throw CadmiumRTClockException("Wait time is negative");
            }

	    //! if duration is infinite (wait forever) then halt and print error over UART
            if (duration == std::numeric_limits<double>::infinity()) {
            	// while (!expired && !interrupted) sleep(); //! exits only if there's an interruption
            	throw CadmiumRTClockException("Wait time is infinite");
            }

	    rTimeLast += duration;
            RealTimeClock::waitUntil(timeNext); //!< equivalent to: vTimeLast = timeNext;

	    //! take the time between consecutive calls to waitUntil()
	    executionTimer.stop();

	    //! substract from duration the time elapsed between succesive calls to waitUntil()  
	    actualDelay = secondsToMicros(duration) - executionTimer.read_us() + schedulerSlip;

	    //! Slip keeps track of how far behind schedule we are.
	    //! In case the time between calls is bigger than duration => schedulerSlip saves the difference (<0)
	    schedulerSlip = actualDelay;

	    //! If we are ahead of schedule, then reset it to zero
	    if (schedulerSlip >= 0) {
            	schedulerSlip = 0;
            }

	    actualDelay = setTimeout(actualDelay);

            if (maxJitter >= 0) {
                auto jitter = actualDelay; 
                if (jitter > maxJitter) {
                    throw cadmium::CadmiumRTClockException("delay jitter is too high"); //! if clock slip is over max allowed jitter then halt and print error over UART
                }
            }

	    executionTimer.reset();
        }

        /**
         * Sets the interrupted flag in case of an external interruption comes up.
         */
	void update() {  
		interrupted = true;
	}
	
        /**
         * Handler for the timeout interrupt.
         */
	void timeoutExpired() {
		expired = true;
	}
    };
}

#endif // CADMIUM_SIMULATION_RT_CLOCK_MBED_HPP
