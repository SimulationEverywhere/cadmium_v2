/**
 * Srijan Gupta & James Grieder
 * ARSLab - Carleton University
 * 
 * A DEVS model for digital output pins on the MSP432P401R board.  This model can be 
 * used for the red LED and RGB on the MSP432P401R, or the RGB on the Educational 
 * Boosterpack MK II.  
 * 
 * Upon receiving a boolean value in the external transition function, the output 
 * pin will be set to high or low.
 */

#ifndef __DIGITAL_OUTPUT_HPP__
#define __DIGITAL_OUTPUT_HPP__

#include <modeling/devs/atomic.hpp>
#ifndef NO_LOGGING
    #include <iostream>
#endif
#include <gpio.h>
#include <rom.h>

namespace cadmium {
    struct DigitalOutputState {
        bool output;
        /**
         * DigitalOutput state constructor.
         * 
         */
        explicit DigitalOutputState(): output(false){}
    };
#ifndef NO_LOGGING
    /**
     * Insertion operator for DigitalOutputState objects.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream.
     */
    std::ostream& operator<<(std::ostream &out, const DigitalOutputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }
#endif
    class DigitalOutput : public Atomic<DigitalOutputState> {
      public:
      
        Port<bool> in;
        //Parameters to be overwriten when instantiating the atomic model
        uint_fast8_t port;
        uint_fast16_t pins;

        // default constructor
        DigitalOutput(const std::string& id, uint_fast8_t selectedPort,uint_fast16_t selectedPins): Atomic<DigitalOutputState>(id, DigitalOutputState())  {
            in = addInPort<bool>("in");
            port = selectedPort;
            pins = selectedPins;
            ROM_GPIO_setAsOutputPin(port,pins);
        };
      
        // internal transition
        void internalTransition(DigitalOutputState& state) const override {
        }

        // external transition
        void externalTransition(DigitalOutputState& state, double e) const override {
            if(!in->empty()){
			    for(const auto x : in->getBag()){
				    state.output = x;
			    }
                state.output ? ROM_GPIO_setOutputHighOnPin(port,pins) : ROM_GPIO_setOutputLowOnPin(port,pins);
		    }
        }
      
        // output function
        void output(const DigitalOutputState& state) const override {
        }

        // time_advance function
        [[nodiscard]] double timeAdvance(const DigitalOutputState& state) const override {
            return std::numeric_limits<double>::infinity();
        }

  };
} // namespace cadmium

#endif // __DIGITAL_OUTPUT_HPP__
