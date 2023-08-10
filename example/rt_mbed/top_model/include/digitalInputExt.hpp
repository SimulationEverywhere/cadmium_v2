/**
* Ezequiel Pecker-Marcosig
* SEDLab - Universidad de Buenos Aires
* 
* Jon Menard
* ARSLab - Carleton University
*
* Digital Input:
* Model to interface with a digital Input pin for Embedded Cadmium.
*/

#ifndef RT_DIGITALINPUTEXT_TEST_HPP
#define RT_DIGITALINPUTEXT_TEST_HPP

#include <optional>
#include <cadmium/modeling/devs/atomic.hpp>

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
	#include "../mbed.h"
#endif

using namespace std;

namespace cadmium {
  
  struct DigitalInputExtState {
      bool output;
      bool last;
      double sigma;

      /**
      * DigitalInputExtState constructor.
      * 
      */
      explicit DigitalInputExtState(): output(true), last(false), sigma(0){}
  }; 

  /**
     * Insertion operator for DigitalInputExtState objects.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream.
     */
    std::ostream& operator<<(std::ostream &out, const DigitalInputExtState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class DigitalInputExt : public Atomic<DigitalInputExtState> {
      public:
      
        Port<bool> out;
#ifndef RT_ARM_MBED
        Port<bool> in;
        //Parameters to be overwriten when instantiating the atomic model
#else
        DigitalIn* digiPin;
        double   pollingRate;
#endif

#ifndef RT_ARM_MBED
        // default constructor
        DigitalInputExt(const std::string& id): Atomic<DigitalInputExtState>(id, DigitalInputExtState())  {
          out = addOutPort<bool>("out");
	  in  = addInPort<bool>("in");
          state.output = 0;
          state.last = state.output;
        };
#else
        // default constructor
        DigitalInputExt(const std::string& id, PinName pin): Atomic<DigitalInputExtState>(id, DigitalInputExtState())  {
          out = addOutPort<bool>("out");
	  digiPin = new DigitalIn(pin);
          pollingRate = 0.10; 
          state.output = digiPin->read();
          state.last = state.output;
        };
#endif
      
      // internal transition
      void internalTransition(DigitalInputExtState& state) const override {

#ifdef RT_ARM_MBED
        state.last = state.output;
	state.output = digiPin->read() == 1; // 0: button pressed, 1: button released
        state.sigma = pollingRate;
#else
        state.sigma = std::numeric_limits<double>::infinity();
#endif
      }

      // external transition
      void externalTransition(DigitalInputExtState& state, double e) const override {
#ifndef RT_ARM_MBED
      	if(!in->empty()){
		for( const auto x : in->getBag()){
			if (x==0)
				state.output = x; // x == 0 -> button pressed
		}
	}
	state.sigma = 0; // automatically generates an internal transition
#endif
      }
      
      // output function
      void output(const DigitalInputExtState& state) const override {
#ifdef RT_ARM_MBED
        if(state.last != state.output) {
          bool output = state.output;
          out->addMessage(output);
        }
#else
	bool output = state.output;
	out->addMessage(output);
#endif
      }

      // time_advance function
      [[nodiscard]] double timeAdvance(const DigitalInputExtState& state) const override {     
          return state.sigma;
      }

  };
} 

#endif // RT_DIGITALINPUTEXT_TEST_HPP
