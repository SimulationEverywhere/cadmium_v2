/**
* Jon Menard
* ARSLab - Carleton University
*
* PWM Input:
* Model to interface with a PWM Output pin for Embedded Cadmium.
*/

#ifndef RT_PWMOUTPUT_HPP
#define RT_PWMOUTPUT_HPP

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
#include "../mbed.h"


using namespace std;

namespace cadmium {
  
  struct PWMOutputState {
      double output;
      double sigma;

      /**
      * Processor state constructor. By default, the processor is idling.
      * 
      */
      explicit PWMOutputState(): output(0), sigma(0){
      }

  }; 

  /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    
    std::ostream& operator<<(std::ostream &out, const PWMOutputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class PWMOutput : public Atomic<PWMOutputState> {
      public:
      
        Port<double> in;
        //Parameters to be overwriten when instantiating the atomic model
        PwmOut* pwnPin;

        // default constructor
        PWMOutput(const std::string& id, PinName pin): Atomic<PWMOutputState>(id, PWMOutputState())  {
          in = addInPort<bool>("in");
          pwmPin = new mbed::PwmOut(pin);
          pwmPin->period_ms(10);
          pwmPin->pulsewidth_ms(0);
        };
      
      // internal transition
      void internalTransition(PWMOutputState& state) const override {
      }

      // external transition
      void externalTransition(PWMOutputState& state, double e) const override {
        if(!in->empty()){
				  for( const auto x : in->getBag()){
					  state.output = x;
				  }

          pwnPin->write(state.output);
			  }
      }
      
      
      // output function
      void output(const PWMOutputState& state) const override {
      };

      // time_advance function
      [[nodiscard]] double timeAdvance(const PWMOutputState& state) const override {     
          return std::numeric_limits<double>::infinity();
      }

  };
}

#endif // RT_PWMOUTPUT_HPP
