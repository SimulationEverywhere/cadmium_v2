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

#ifndef CADMIUM_RT_CLOCK_LINUX_HPP
#define CADMIUM_RT_CLOCK_LINUX_HPP

#include <chrono>
#include <iostream>
#include "cadmium/simulation/rt/real_time/linux/asynchronous_events.hpp"

static long MIN_TO_MICRO   = (1000*1000*60);
static long SEC_TO_MICRO   = (1000*1000);
static long MILI_TO_MICRO  = (1000);

#ifndef MISSED_DEADLINE_TOLERANCE
  #define MISSED_DEADLINE_TOLERANCE 500
#endif
// extern volatile bool interrupted;
bool interrupted = false;
namespace cadmium {

        class Timer
        {
          std::chrono::high_resolution_clock::time_point start_time, end_time;
        public:
          Timer():
            start_time( std::chrono::high_resolution_clock::now()){}

          void start() {
            start_time = std::chrono::high_resolution_clock::now();
          }
          void reset() {
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
          Timer execution_timer;

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
            execution_timer.reset();
            execution_timer.start();
            //Wait until timeout or interrupt
            while(!interrupted && (delay_us > execution_timer.live_read_us()));
            
            execution_timer.stop();
            if(interrupted) {
              std::cout << "We were interupted";
              return delay_us - execution_timer.read_us();
            }
            return delay_us;
          }

       public:

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
              while(1); //Sleep
            }

            execution_timer.stop();
            actual_delay = get_time_in_micro_seconds(t) - execution_timer.read_us() + scheduler_slip;
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

            execution_timer.reset();
            execution_timer.start();

            return 0;
          }
          
          void update(){
            interrupted = true;
          }

          void startSimulation(){
            execution_timer.reset();
            execution_timer.start();
          }
        };
}

#endif //CADMIUM_RT_CLOCK_LINUX_HPP