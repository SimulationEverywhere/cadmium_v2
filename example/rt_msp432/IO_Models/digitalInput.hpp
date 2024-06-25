/**
 * Srijan Gupta & James Grieder
 * ARSLab - Carleton University
 * 
 * A DEVS model for digital input pins on the MSP432P401R board.  This model can be 
 * used for switch inputs on the MSP432P401R, and switch and gator hole inputs on 
 * the Educational Boosterpack MK II.  
 * 
 * The specified pins are polled every 0.1 seconds, and will output the pin status 
 * as a boolean.
 */

#ifndef __DIGITAL_INPUT_HPP__
#define __DIGITAL_INPUT_HPP__

#include <modeling/devs/atomic.hpp>
#include <gpio.h>
#include <rom.h>
#ifndef NO_LOGGING
    #include <iostream>
#endif

namespace cadmium {
    struct DigitalInputState {
        bool output;
        bool last;
        double sigma;

        /**
        * DigitalInputState constructor.
        */
        explicit DigitalInputState(): output(true), last(false), sigma(0){}
    };

#ifndef NO_LOGGING
    /**
     * Insertion operator for DigitalInputState objects.
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream.
     */
    std::ostream& operator<<(std::ostream &out, const DigitalInputState& state) {
        out << "Pin: " << (state.output ? 1 : 0); 
        return out;
    }
#endif
    class DigitalInput : public Atomic<DigitalInputState> {
      public:
      
        Port<bool> out;

        //Parameters to be overwritten when instantiating the atomic model
        uint_fast8_t port;
        uint_fast16_t pins;

        // default constructor
        DigitalInput(const std::string& id, uint_fast8_t selectedPort, uint_fast16_t selectedPins): Atomic<DigitalInputState>(id, DigitalInputState())  {
            out = addOutPort<bool>("out");
            port = selectedPort;
            pins = selectedPins;

            if (port == GPIO_PORT_P2 && pins == GPIO_PIN3) {
                ROM_GPIO_setAsInputPin(port, pins);
            } else {
                ROM_GPIO_setAsInputPinWithPullUpResistor(port,pins);
            }

            state.output = ROM_GPIO_getInputPinValue(port,pins);
            state.last = state.output;
            state.sigma = 0.1;
        };
      
        // internal transition
        void internalTransition(DigitalInputState& state) const override {
            state.last = state.output;
            state.output = ROM_GPIO_getInputPinValue(port,pins);
        }

        // external transition
        void externalTransition(DigitalInputState& state, double e) const override {
            throw CadmiumSimulationException("External transition called in a model with no input ports");
        }
      
        // output function
        void output(const DigitalInputState& state) const override {
            if(state.last != state.output) {
                out->addMessage(state.output);
            }
        }

        // time_advance function
        [[nodiscard]] double timeAdvance(const DigitalInputState& state) const override {
            return state.sigma;
        }
    };
} // namespace cadmium

#endif // __DIGITAL_INPUT_HPP__
