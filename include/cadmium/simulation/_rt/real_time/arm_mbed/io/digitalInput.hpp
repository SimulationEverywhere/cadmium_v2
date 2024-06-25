/**
* Jon Menard
* ARSLab - Carleton University
*
* Digital Input:
* Model to interface with a digital Input pin for Embedded Cadmium.
*/

#ifndef RT_DIGITALINPUT_TEST_HPP
#define RT_DIGITALINPUT_TEST_HPP

#include <iostream>
#include <optional>
#include "cadmium/modeling/devs/atomic.hpp"

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
  
  struct DigitalInputState {
      bool output;
      bool last;
      double sigma;

      /**
      * Processor state constructor. By default, the processor is idling.
      * 
      */
      explicit DigitalInputState(): output(true), last(false), sigma(0){
      }

  }; 

  /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    
    std::ostream& operator<<(std::ostream &out, const DigitalInputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class DigitalInput : public Atomic<DigitalInputState> {
      public:
      
        Port<bool> out;
        //Parameters to be overwriten when instantiating the atomic model
        DigitalIn* digiPin;
        double   pollingRate;
        // default constructor
        DigitalInput(const std::string& id, PinName pin): Atomic<DigitalInputState>(id, DigitalInputState())  {
          out = addOutPort<bool>("out");
          digiPin = new DigitalIn(pin);
          pollingRate = 0.10; 
          state.output = digiPin->read();
          state.last = state.output;
        };
      
      // internal transition
      void internalTransition(DigitalInputState& state) const override {
        state.last = state.output;
        state.output = digiPin->read() == 1;
        state.sigma = pollingRate;
      }

      // external transition
      void externalTransition(DigitalInputState& state, double e) const override {
        // MBED_ASSERT(false);
        // throw std::logic_error("External transition called in a model with no input ports");
      }
      
      // output function
      void output(const DigitalInputState& state) const override {
        if(state.last != state.output) {
          bool output = state.output;
          out->addMessage(output);
        }
      }

      // time_advance function
      [[nodiscard]] double timeAdvance(const DigitalInputState& state) const override {     
          return state.sigma;
      }

  };
} 

#endif // BOOST_SIMULATION_PDEVS_DIGITALINPUT_HPP
