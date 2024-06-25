/**
* Jon Menard
* ARSLab - Carleton University
*
* Interrupt Input:
* Model to interface with a Interrupt Input pin for Embedded Cadmium.
*/

#ifndef RT_InterruptINPUT_TEST_HPP
#define RT_InterruptINPUT_TEST_HPP

#include <iostream>
#include <optional>
#include "cadmium/modeling/devs/atomic.hpp"
#include "cadmium/simulation/_rt/real_time/linux/asynchronous_events.hpp"

#include <limits>
#include <math.h>
#include <assert.h>
#include <memory>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>

#ifdef RT_ARM_MBED
#endif

#include "../mbed.h"
using namespace std;

namespace cadmium {
  
  void AsyncEvent::notify() {
      interrupted = true;
      for (unsigned int i = 0; i < views.size(); i++)
          views[i]->update();
  }

  

  struct InterruptInputState {
      bool output;
      bool last;
      double sigma;

      /**
      * Processor state constructor. By default, the processor is idling.
      * 
      */
      explicit InterruptInputState(): output(true), last(false), sigma(0){
      }

  }; 

  /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    
    std::ostream& operator<<(std::ostream &out, const InterruptInputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class InterruptInput : public Atomic<InterruptInputState>, public AsyncEvent {
      public:
      
        Port<bool> out;
        Port<bool> triggerExternal;
        //Parameters to be overwriten when instantiating the atomic model
        InterruptIn* interruptPin;

        // default constructor
        InterruptInput(const std::string& id, PinName pin): Atomic<InterruptInputState>(id, InterruptInputState()), AsyncEvent(){
          out = addOutPort<bool>("out");
          triggerExternal = addInPort<bool>("triggerExternal");
          setPort(triggerExternal);
          interruptPin = new InterruptIn(pin);
          interruptPin->rise(callback(this, &cadmium::AsyncEvent::notify));
          interruptPin->fall(callback(this, &cadmium::AsyncEvent::notify));
          state.output = interruptPin->read();
          state.last = state.output;
        };
      
      
      // internal transition
      void internalTransition(InterruptInputState& state) const override {
        state.sigma = std::numeric_limits<double>::infinity();
      }

      // external transition
      void externalTransition(InterruptInputState& state, double e) const override {
        // if(!triggerExternal->empty()){

          state.sigma = 0;
          state.last = state.output;
          state.output = (interruptPin->read() == 1);
				  
			  // }
        
        // MBED_ASSERT(false);
        // throw std::logic_error("External transition called in a model with no input ports");
      }
      
      // output function
      void output(const InterruptInputState& state) const override {
          // if(state.last != state.output)
          out->addMessage(state.output);    
      }

      // time_advance function
      [[nodiscard]] double timeAdvance(const InterruptInputState& state) const override {     
        return state.sigma;
      }

      void confluentTransition(InterruptInputState& s, double e) const {
            this->externalTransition(s, e);
            this->internalTransition(s);
      }

  };
} 

#endif // BOOST_SIMULATION_PDEVS_InterruptINPUT_HPP
