/**
* Jon Menard
* ARSLab - Carleton University
*
* Digital Output:
* Model to interface with a digital Output pin for Embedded Cadmium.
*/

#ifndef RT_DIGITALOUTPUT_TEST_HPP
#define RT_DIGITALOUTPUT_TEST_HPP

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
  
  struct DigitalOutputState {
      bool output;
      double sigma;

      /**
      * Processor state constructor. By default, the processor is idling.
      * 
      */
      explicit DigitalOutputState(): output(true), sigma(0){
      }

  }; 

  /**
     * Insertion operator for ProcessorState objects. It only displays the value of sigma.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream with sigma already inserted.
     */
    
    std::ostream& operator<<(std::ostream &out, const DigitalOutputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class DigitalOutput : public Atomic<DigitalOutputState> {
      public:
      
        Port<bool> in;
        //Parameters to be overwriten when instantiating the atomic model
        DigitalOut* digiPin;

        // default constructor
        DigitalOutput(const std::string& id, PinName pin): Atomic<DigitalOutputState>(id, DigitalOutputState())  {
          in = addInPort<bool>("in");
          digiPin = new DigitalOut(pin);
        };
      
      // internal transition
      void internalTransition(DigitalOutputState& state) const override {
      }

      // external transition
      void externalTransition(DigitalOutputState& state, double e) const override {
        if(!in->empty()){
				  for( const auto x : in->getBag()){
					  state.output = x;
				  }

          digiPin->write(state.output ? 1 : 0);
			  }
      }
      
      
      // output function
      void output(const DigitalOutputState& state) const override {
      };

      // time_advance function
      [[nodiscard]] double timeAdvance(const DigitalOutputState& state) const override {     
          return std::numeric_limits<double>::infinity();
      }

  };
}

#endif // RT_DIGITALOUTPUT_TEST_HPP
