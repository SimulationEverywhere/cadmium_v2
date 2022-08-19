/**
 * Copyright (c) 2019, Kyle Bjornson, Ben Earle
 * Carleton University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CADMIUM_RT_CLOCK_HPP
#define CADMIUM_RT_CLOCK_HPP


#include <mbed.h>
#include <chrono>
#include <iostream>
#include <cadmium/core/real_time/linux/asynchronous_events.hpp>
#include "../exception.hpp"
// #include "./asynchronous_atomic.hpp"


static long MIN_TO_MICRO   = (1000*1000*60);
static long SEC_TO_MICRO   = (1000*1000);
static long MILI_TO_MICRO  = (1000);

#ifndef MISSED_DEADLINE_TOLERANCE
  #define MISSED_DEADLINE_TOLERANCE 500
#endif
// extern volatile bool interrupted;

namespace cadmium {

        #ifndef RT_ARM_MBED
          class Timer{
            std::chrono::high_resolution_clock::time_point start_time, end_time;
            public:
              Timer():
                start_time( std::chrono::high_resolution_clock::now()){}

              void start() {
                start_time = std::chrono::high_resolution_clock::now();
              }
              void reset() {
                start();
                end_time = start_time;
              }
              void stop(){
                end_time = std::chrono::high_resolution_clock::now();
              }
              long live_read_us(){
                return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start_time).count() * SEC_TO_MICRO;            
              }
              long read_us(){
                return std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count() * SEC_TO_MICRO;
              }
          };
        #endif

        /**
         * @brief Wall Clock class used to delay execution and follow actual time.
         * Used mbed timeout, and attempts to sleep the main thread to save some power.
         */
        // template<class TIME, typename LOGGER=cadmium::logger::logger<cadmium::logger::logger_debug,
        //                                      cadmium::dynamic::logger::formatter<TIME>,
        //                                      cadmium::logger::cout_sink_provider>>
        
        
        class RTClock : public AsyncEventObserver {
        private:

          
          //Time since last time advance, how long the simulator took to advance
          Timer executionTimer;
          Timeout _timeout; //mbed timeout object
          bool expired;

          // If the next event (actual_delay) is in the future AKA we are ahead of schedule it will be reset to 0
          // If actual_delay is negative we are behind schedule, in this case we will store how long behind schedule we are in scheduler_slip.
          // This is then added to the next actual delay and updated until we surpass the tolerance or recover from the slip.
          long scheduler_slip = 0;

          //Return a long of time in microseconds
          long get_time_in_micro_seconds(const double t) const {

            //Ignore Anything below 1 microsecond
            return t * SEC_TO_MICRO;
          }

          double micro_seconds_to_time(long us) const {
            return us / SEC_TO_MICRO;
          }

          //Given a long in microseconds, sleep for that time
        
          long set_timeout(long delay_us) {
            expired = false;
            long timeLeft = delay_us;
            executionTimer.reset();
            executionTimer.start();

            //Handle waits of over ~35 minutes as timer overflows
            while ((timeLeft > INT_MAX) && !interrupted) {
              this->expired = false;
              this->_timeout.attach_us(callback(this, &RTClock::timeout_expired), INT_MAX);

              while (!expired && !interrupted) sleep();

              if(!interrupted){
                timeLeft -= INT_MAX;
              }
            }

            //Handle waits of under INT_MAX microseconds
            if(!interrupted && timeLeft > 0) {
              this->_timeout.attach_us(callback(this, &RTClock::timeout_expired), timeLeft);
              while (!expired && !interrupted) sleep();
            }

            executionTimer.stop();
            expired = false;
            if(interrupted) {
              timeLeft -= executionTimer.read_us();
              if(delay_us < timeLeft ) return 0;
              //hal_critical_section_enter();
              // interrupted = false;
              return delay_us - timeLeft;
            }
            return 0;
          }

       public:

          

          volatile bool interrupted;


          RTClock(std::vector<std::shared_ptr<AsyncEvent>> asyncSubjects): AsyncEventObserver(asyncSubjects){
              interrupted = false;
          }
          
          double wait_for(const double t) {
            long actual_delay;

            //If negative time, halt and print error over UART
            if(t < 0){
              throw CadmiumSimulationException("Time is negative - rt_clock.hpp");
            }

            //Wait forever
            if (t == std::numeric_limits<double>::infinity()) {
              while (!expired && !interrupted) sleep();
            }

            executionTimer.stop();
            actual_delay = get_time_in_micro_seconds(t) - executionTimer.read_us() + scheduler_slip;
            // Slip keeps track of how far behind schedule we are.
            scheduler_slip = actual_delay;
            // If we are ahead of schedule, then reset it to zero
            if (scheduler_slip >= 0) {
              scheduler_slip = 0;
            }

            if (MISSED_DEADLINE_TOLERANCE != -1 ) {
              if (actual_delay >= -MISSED_DEADLINE_TOLERANCE) {
                actual_delay = set_timeout(actual_delay);
              } else {
                //Missed Real Time Deadline and could not recover (Slip is passed the threshold)
                std::cout << "MISSED SCHEDULED TIME ADVANCE DEADLINE BY:" << -actual_delay << " microseconds \n";
                throw CadmiumSimulationException("MISSED SCHEDULED TIME ADVANCE DEADLINE - rt_clock.hpp");
              }
            }

            executionTimer.reset();
            executionTimer.start();

            return micro_seconds_to_time(actual_delay);
          }
          
          void update(){
            interrupted = true;
          }

          void timeout_expired() {
            expired = true;
          }

          void startSimulation(){
            executionTimer.reset();
            executionTimer.start();
          }
        };
}

#endif //CADMIUM_RT_CLOCK_HPP
