/**
 * A DEVS interface for the digital output on the MSP432P401R board
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

#ifndef DIGITAL_OUTPUT_HPP
#define DIGITAL_OUTPUT_HPP

#include <cadmium/modeling/devs/atomic.hpp>
#include <gpio.h>
#include <rom.h>
#ifndef NO_LOGGING
    #include <iostream>
#endif

namespace cadmium {
    struct DigitalOutputState {
        bool output;
        /**
         * Constructor.
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
        uint_fast8_t port;
        uint_fast16_t pins;

        // constructor
        DigitalOutput(const std::string& id, uint_fast8_t selectedPort,uint_fast16_t selectedPins): Atomic<DigitalOutputState>(id, DigitalOutputState())  {
            in = addInPort<bool>("in");
            port = selectedPort;
            pins = selectedPins;
            ROM_GPIO_setAsOutputPin(port,pins);
        }
      
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
      
        // output
        void output(const DigitalOutputState& state) const override {
        }

        // time advance
        [[nodiscard]] double timeAdvance(const DigitalOutputState& state) const override {
            return std::numeric_limits<double>::infinity();
        }

  };
} // namespace cadmium

#endif // DIGITAL_OUTPUT_HPP
