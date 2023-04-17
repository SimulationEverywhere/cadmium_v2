/**
 * Real-time clock based on the Driver APIs from MBED-OS version 5.
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

#include <iostream>
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
	// executionTimer is used in the waitUntil() to measure the elapsed time between cnsecutive calls from the Root Coordinator and the time elapsed between an event and an external interruption
	Timer executionTimer;
	Timeout _timeout;
	bool expired;

        // scheduler_slip = 0 if the next event (actual_delay) is in the future, AKA we are ahead of schedule
        // scheduler_slip = <how_long_behind_schedule_we_are> if actual_delay is negative, i.e. we are behind schedule.
        // This is then added to the next actual_delay and updated until we surpass the tolerance or recover from the slip.
        long scheduler_slip = 0;

	// double e; // elapsed time until interrupt

        // 
        /**
         * Returns a long with the time in microseconds.
         * @param Time value in seconds.
         */
        long get_time_in_micro_seconds(const double t) const {
        	//Ignore Anything below 1 microsecond
        	return t * SEC_TO_MICRO;
        }

        /**
         * Returns a double with the time in seconds.
         * @param Time in microseconds.
         */
        double micro_seconds_to_time(long us) const {
        	return us / SEC_TO_MICRO;
        }

        /**
         * Sleeps during specified time in microseconds. 
         * @param timeLeft time to sleep (in microseconds).
         */
        // 
        long set_timeout(long delay_us) {
        	expired = false;
          	long timeLeft = delay_us;
		printf("[set_timeout] timeLeft = %ld us\n",timeLeft);

		executionTimer.reset();

        	// Handle waits of over ~35 minutes as timer overflows
        	while (!interrupted && (timeLeft > INT_MAX)) {
        	 	this->expired = false;
        	 	this->_timeout.attach_us(callback(this, &MBEDClock::timeout_expired), INT_MAX);

        	 	while (!expired && !interrupted) {
				sleep();
			}

        	 	if(!interrupted){
        	 		timeLeft -= INT_MAX;
        	 	}
        	}

        	//Handle waits of under INT_MAX microseconds
        	if(!interrupted && timeLeft > 0) {
        		this->_timeout.attach_us(callback(this, &MBEDClock::timeout_expired), timeLeft); // sets callback to change expire after timeLeft time
        		while (!expired && !interrupted) {
				sleep(); // remains here until expired is set true by the _timeout callback
			}
        	}

		// if timeLeft < 0 there is no wait

        	executionTimer.stop();
        	expired = false;

		// in case the sleep() is interrupted by an external interrupt return the remaining time, otherwise return 0
        	if(interrupted) {
        	 	timeLeft -= executionTimer.read_us();
        	 	if(delay_us < timeLeft ) {
				return 0;
			}
        	 	//hal_critical_section_enter();
        	 	// interrupted = false;
        	 	return delay_us - timeLeft;
        	}

        	return 0;
        }

     protected:
        T rTimeLast; //!< last real system time.
        long  maxJitter; //!< Maximum allowed delay jitter. This parameter is optional.
     public:
        volatile bool interrupted; // for external interrupts

        //! The empty constructor does not check the accumulated delay jitter.
        MBEDClock(): RealTimeClock(), executionTimer(), _timeout() {
		rTimeLast = 0.0;
		maxJitter = -1;
		expired = false;
		interrupted = false;// used for interrupts
	}

	/**
	 * Use this constructor to select the maximum allowed delay jitter.
	 * @param maxJitter duration of the maximum allowed jitter.
	 */
	[[maybe_unused]] explicit MBEDClock(long max_jitter): MBEDClock() {
		maxJitter = max_jitter;
		printf("[MBEDClock Constructor] maxJitter = %ld\n",maxJitter);
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
	    long actual_delay;
	    //If negative time, halt and print error over UART
	    if(duration < 0) {
            	throw CadmiumRTClockException("Wait time is negative");
            }

	    //Wait forever
            if (duration == std::numeric_limits<double>::infinity()) {
            	// while (!expired && !interrupted) sleep(); // exits only if there's an interruption
            	throw CadmiumRTClockException("Wait time is infinite");
            }
	    rTimeLast += duration;
            RealTimeClock::waitUntil(timeNext);
	    // take the time between consecutive calls to waitUntil()
	    executionTimer.stop();
	    // substract from duration the time elapsed between succesive calls to waitUntil()  
	    actual_delay = get_time_in_micro_seconds(duration)-executionTimer.read_us() + scheduler_slip;
	    // Slip keeps track of how far behind schedule we are.
	    // In case the time between calls is bigger than duration => scheduler_slip saves the difference (<0)
	    scheduler_slip = actual_delay;
	    // If we are ahead of schedule, then reset it to zero
	    if (scheduler_slip >= 0) {
            	scheduler_slip = 0;
            }

	    printf("[waitUntil] actual_delay = %ld us\n",actual_delay);
	    actual_delay = set_timeout(actual_delay);

            if (maxJitter >= 0) {
                auto jitter = actual_delay; 
                if (jitter > maxJitter) {
                    throw cadmium::CadmiumRTClockException("delay jitter is too high");
                }
            }

	    executionTimer.reset();
        }

        /**
         * Sets the interrupted flag in case of an external interruption comes up.
         * @param
         */
	void update() { // 
		interrupted = true;
	}
	
        /**
         * Handler for the timeout interrupt.
         * @param 
         */
	void timeout_expired() {
		expired = true;
	}
    };
}

#endif // CADMIUM_SIMULATION_RT_CLOCK_MBED_HPP
