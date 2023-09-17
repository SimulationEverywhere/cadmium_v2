/**
 * A DEVS interface for the digital input on the MSP432P401R board
 *
 * Copyright (C) 2023 James Grieder & Srijan Gupta
 * ARSLab - Carleton University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DIGITAL_INPUT_HPP
#define DIGITAL_INPUT_HPP

#include <cadmium/modeling/devs/atomic.hpp>
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
         * Constructor.
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
        out << "Input: " << state.output; 
        return out;
    }
#endif

    class DigitalInput : public Atomic<DigitalInputState> {
      public:
        Port<bool> out;
        uint_fast8_t port;
        uint_fast16_t pins;

        // constructor
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
        }
      
        // output
        void output(const DigitalInputState& state) const override {
            if(state.last != state.output) {
                bool output = state.output;
                out->addMessage(output);
            }
        }

        // time advance
        [[nodiscard]] double timeAdvance(const DigitalInputState& state) const override {
            return state.sigma;
        }
    };
} // namespace cadmium

#endif // DIGITAL_INPUT_HPP
