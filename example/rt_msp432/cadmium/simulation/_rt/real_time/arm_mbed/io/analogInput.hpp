/**
* Jon Menard
* ARSLab - Carleton University
*
* Analog Input:
* Model to interface with a Analog Input pin for Embedded Cadmium.
*/

#ifndef RT_ANALOGINPUT_HPP
#define RT_ANALOGINPUT_HPP

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
  
  struct AnalogInputState {
      double output;
      double last;
      double sigma;

      /**
      * Processor state constructor. By default, the processor is idling.
      * 
      */
      explicit AnalogInputState(): output(0), last(0), sigma(0){
      }

  }; 

  /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    
    std::ostream& operator<<(std::ostream &out, const AnalogInputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class AnalogInput : public Atomic<AnalogInputState> {
      public:
      
        Port<double> out;
        //Parameters to be overwriten when instantiating the atomic model
        AnalogIn* analogPin;
        double   pollingRate;
        // default constructor
        AnalogInput(const std::string& id, PinName pin): Atomic<AnalogInputState>(id, AnalogInputState())  {
          out = addOutPort<bool>("out");
          analogPin = new AnalogIn(pin);
          pollingRate = 0.01; 
          state.output = (double) analogPin->read();
          state.last = state.output;
        };
      
      // internal transition
      void internalTransition(AnalogInputState& state) const override {
        state.last = state.output;
        state.output = (double) analogPin->read();
        state.sigma = pollingRate;
      }

      // external transition
      void externalTransition(AnalogInputState& state, double e) const override {
        // MBED_ASSERT(false);
        // throw std::logic_error("External transition called in a model with no input ports");
      }
      
      // output function
      void output(const AnalogInputState& state) const override {
        if(state.last != state.output) {
          out->addMessage(state.output);
        }
      }

      // time_advance function
      [[nodiscard]] double timeAdvance(const AnalogInputState& state) const override {     
          return state.sigma;
      }
  };
} 

#endif // RT_ANALOGINPUT_HPP
