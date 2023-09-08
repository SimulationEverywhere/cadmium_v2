/**
 * A DEVS interface for the Joystick on the Educational Boosterpack MK II
 * when used with a MSP432P401R board.  This includes the Horizontal and Vertical
 * Axes of the Joystick, and the SELECT button.
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

#ifndef JOYSTICK_INPUT_HPP
#define JOYSTICK_INPUT_HPP

//*****************************************************************************
// ADC14 Registers
//*****************************************************************************
/* ADC Control Registers */
#define ADC14CTL0   (HWREG32(0x40012000))
#define ADC14CTL1   (HWREG32(0x40012004))
/* ADC Conversion Memory Control Registers */
#define ADC14MCTL0  (HWREG32(0x40012018))
#define ADC14MCTL1  (HWREG32(0x4001201C))
/* ADC Conversion Memory Registers */
#define ADC14MEM0   (HWREG32(0x40012098))
#define ADC14MEM1   (HWREG32(0x4001209C))
/* Interrupt Enable Registers */
#define ADC14IER0   (HWREG32(0x4001213C))
#define ADC14IER1   (HWREG32(0x40012140))
/* Interrupt Flag Registers */
#define ADC14IFGR0  (HWREG32(0x40012144))

#include <cadmium/modeling/devs/atomic.hpp>
#include <gpio.h>
#ifndef NO_LOGGING
    #include <iostream>
#endif

// ------------BSP_Joystick_Input------------
// Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
// Read and return the immediate status of the
// joystick.  Button de-bouncing for the Select
// button is not considered.  The joystick X- and
// Y-positions are returned as 10-bit numbers,
// even if the ADC on the LaunchPad is more precise.
// Input: x is pointer to store X-position (0 to 1023)
//        y is pointer to store Y-position (0 to 1023)
//        select is pointer to store Select status (0 if pressed)
// Output: none
// Assumes: BSP_Joystick_Init() has been called
#define SELECT    (*((volatile uint8_t *)(0x42000000+32*0x4C21+4*1)))
void BSP_Joystick_Input(uint16_t *x, uint16_t *y, uint8_t *select){
    ADC14CTL0 &= ~0x00000002;             // 1) ADC14ENC = 0 to allow programming
    while(ADC14CTL0&0x00010000){};        // 2) wait for BUSY to be zero
    ADC14CTL1 = (ADC14CTL1&~0x001F0000) | // clear STARTADDx bit field
            (0 << 16);                // 3) configure for STARTADDx = 0
    ADC14CTL0 |= 0x00000002;              // 4) enable conversions
    while(ADC14CTL0&0x00010000){};        // 5) wait for BUSY to be zero
    ADC14CTL0 |= 0x00000001;              // 6) start single conversion
    while((ADC14IFGR0&0x02) == 0){};      // 7) wait for ADC14IFG1
    *x = (int)ADC14MEM0>>4;               // 8) P6.0/A15 result 0 to 1023
    *y = (int)ADC14MEM1>>4;               //    P4.4/A9 result 0 to 1023
    *select = (int)SELECT;                // return 0(pressed) or 0x01(not pressed)
}

namespace cadmium {
    struct JoystickInputState {
        uint16_t outputX;
        uint16_t outputY;
        uint8_t outputSelect;
        uint16_t lastX;
        uint16_t lastY;
        uint8_t lastSelect;
        double sigma;
        /**
         * Constructor
         */
        explicit JoystickInputState(): outputX(0), outputY(0), outputSelect(0), lastX(0), lastY(0), lastSelect(0), sigma(0) {}
    };

#ifndef NO_LOGGING
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
#endif

    class JoystickInput : public Atomic<JoystickInputState> {
      public:
        Port<uint16_t> outX;
        Port<uint16_t> outY;
        Port<uint8_t> outSelect;

        // default constructor
        JoystickInput(const std::string& id): Atomic<JoystickInputState>(id, JoystickInputState())  {

            outX = addOutPort<uint16_t>("outX");
            outY = addOutPort<uint16_t>("outY");
            outSelect = addOutPort<uint8_t>("outSelect");

            // Initialize ADC
            // Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
            ADC14CTL0 &= ~0x00000002;        // 2) ADC14ENC = 0 to allow programming
            while(ADC14CTL0&0x00010000){};   // 3) wait for BUSY to be zero
            ADC14CTL0 = 0x04223390;          // 4) single, SMCLK, on, disabled, /1, 32 SHM
            ADC14CTL1 = 0x00000030;          // 5) ADC14MEM0, 14-bit, ref on, regular power
            // 6) different for each initialization function
            ADC14IER0 = 0;
            ADC14IER1 = 0;                   // 7) no interrupts

            // Initialize Joystick Pins
            // Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
            ADC14MCTL0 = 0x0000000F;         // 6a) 0 to 3.3V, channel 15
            ADC14MCTL1 = 0x00000089;         // 6b) 0 to 3.3V, channel 9
            P6SEL0 |= 0x01;
            P6SEL1 |= 0x01;                  // 8a) analog mode on P6.0/A15
            P4SEL0 |= 0x10;
            P4SEL1 |= 0x10;                  // 8b) analog mode on P4.4/A9
            ADC14CTL0 |= 0x00000002;         // 9) enable
            P4SEL0 &= ~0x02;
            P4SEL1 &= ~0x02;                 // configure P4.1 as GPIO
            P4DIR &= ~0x02;                  // make P4.1 in
            P4REN &= ~0x02;                  // disable pull resistor on P4.1


            // Get an initial reading
            BSP_Joystick_Input(&state.outputX, &state.outputY, &state.outputSelect);

            state.lastX = state.outputX;
            state.lastY = state.outputY;
            state.lastSelect = state.outputSelect;
            state.sigma = 0.1;
        };

        // internal transition
        void internalTransition(JoystickInputState& state) const override {
            state.lastX = state.outputX;
            state.lastY = state.outputY;
            state.lastSelect = state.outputSelect;

            // Read pin
            BSP_Joystick_Input(&state.outputX, &state.outputY, &state.outputSelect);
        }

        // external transition
        void externalTransition(JoystickInputState& state, double e) const override {
        }

        // output
        void output(const JoystickInputState& state) const override {
            if(state.lastX != state.outputX) {
                outX->addMessage(state.outputX);
            }
            if(state.lastY != state.outputY) {
                outY->addMessage(state.outputY);
            }
            if(state.lastSelect != state.outputSelect) {
                outSelect->addMessage(state.outputSelect);
            }
        }

        // time advance
        [[nodiscard]] double timeAdvance(const JoystickInputState& state) const override {
            return state.sigma;
        }
    };
} // namespace cadmium

#endif // JOYSTICK_INPUT_HPP
