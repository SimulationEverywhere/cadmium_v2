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
#include <cadmium/core/modeling/atomic.hpp>
#include "./digitPin.hpp"

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

// #ifdef RT_ARM_MBED
  //This class will interface with a digital input pin.
  //#include "../mbed.h"

// using namespace cadmium;
// using namespace std;
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
        out << "Input Pin: " << (state.output ? 1 : 0); 
        return out;
    }

  class DigitalInput : public Atomic<DigitalInputState> {
      public:
      
        Port<bool> out;
        //Parameters to be overwriten when instantiating the atomic model
        DigitalPin* digiPin;
        double   pollingRate;
        // default constructor
        DigitalInput(const std::string& id): Atomic<DigitalInputState>(id, DigitalInputState())  {
          out = addOutPort<bool>("out");
          digiPin = new DigitalPin();
          pollingRate = 0.01; 
        };
      
      // internal transition
      void internalTransition(DigitalInputState& state) const override {
        state.last = state.output;
        if(digiPin->checkPin()){
          state.output = !state.last;
        }
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
          if(output){
            std::cout << "Input: 1\n";
          }else{
            std::cout << "Input: 0 \n";
          }
        }
      }

      // time_advance function
      [[nodiscard]] double timeAdvance(const DigitalInputState& state) const override {     
          return state.sigma;
      }

  };
} 

#endif // BOOST_SIMULATION_PDEVS_DIGITALINPUT_HPP