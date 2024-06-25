/**
* Jon Menard
* ARSLab - Carleton University
*
* Analog Input:
* Model to interface with a Analog Output pin for Embedded Cadmium.
*/

#ifndef RT_ANALOGOUTPUT_HPP
#define RT_ANALOGOUTPUT_HPP

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
  
  struct AnalogOutputState {
      double output;
      double sigma;

      /**
      * Processor state constructor. By default, the processor is idling.
      * 
      */
      explicit AnalogOutputState(): output(0), sigma(0){
      }

  }; 

  /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    
    std::ostream& operator<<(std::ostream &out, const AnalogOutputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class AnalogOutput : public Atomic<AnalogOutputState> {
      public:
      
        Port<double> in;
        //Parameters to be overwriten when instantiating the atomic model
        AnalogOut* analogPin;

        // default constructor
        AnalogOutput(const std::string& id, PinName pin): Atomic<AnalogOutputState>(id, AnalogOutputState())  {
          in = addInPort<bool>("in");
          analogPin = new mbed::AnalogOut(pin);
        };
      
      // internal transition
      void internalTransition(AnalogOutputState& state) const override {
      }

      // external transition
      void externalTransition(AnalogOutputState& state, double e) const override {
        if(!in->empty()){
				  for( const auto x : in->getBag()){
					  state.output = x;
				  }

          analogPin->write(state.output);
			  }
      }
      
      
      // output function
      void output(const AnalogOutputState& state) const override {
      };

      // time_advance function
      [[nodiscard]] double timeAdvance(const AnalogOutputState& state) const override {     
          return std::numeric_limits<double>::infinity();
      }

  };
}

#endif // RT_ANALOGOUTPUT_HPP
