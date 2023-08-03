/**
 * A DEVS interface for the Accelerometer on the Educational Boosterpack MK II
 * when used with a MSP432P401R board.  This includes X, Y, and Z axes of the
 * accelerometer.
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

#ifndef ACCELEROMETER_INPUT_HPP
#define ACCELEROMETER_INPUT_HPP

//*****************************************************************************
// ADC14 Registers
//*****************************************************************************
/* ADC Control Registers */
#define ADC14CTL0   (HWREG32(0x40012000))
#define ADC14CTL1   (HWREG32(0x40012004))
/* ADC Conversion Memory Control Registers */
#define ADC14MCTL2  (HWREG32(0x40012020))
#define ADC14MCTL3  (HWREG32(0x40012024))
#define ADC14MCTL4  (HWREG32(0x40012028))
/* ADC Conversion Memory Registers */
#define ADC14MEM2   (HWREG32(0x400120A0))
#define ADC14MEM3   (HWREG32(0x400120A4))
#define ADC14MEM4   (HWREG32(0x400120A8))
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

// ------------BSP_Accelerometer_Input------------
// Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
// Read and return the immediate status of the
// accelerometer.  The accelerometer X-, Y-, and
// Z-measurements are returned as 10-bit numbers,
// even if the ADC on the LaunchPad is more precise.
// Input: x is pointer to store X-measurement (0 to 1023)
//        y is pointer to store Y-measurement (0 to 1023)
//        z is pointer to store Z-measurement (0 to 1023)
// Output: none
// Assumes: BSP_Accelerometer_Init() has been called
void BSP_Accelerometer_Input(uint16_t *x, uint16_t *y, uint16_t *z){
    ADC14CTL0 &= ~0x00000002;        // 1) ADC14ENC = 0 to allow programming
    while(ADC14CTL0&0x00010000){};   // 2) wait for BUSY to be zero
    ADC14CTL1 = (ADC14CTL1&~0x001F0000) | // clear STARTADDx bit field
            (2 << 16);           // 3) configure for STARTADDx = 2
    ADC14CTL0 |= 0x00000002;         // 4) enable conversions
    while(ADC14CTL0&0x00010000){};   // 5) wait for BUSY to be zero
    ADC14CTL0 |= 0x00000001;         // 6) start single conversion
    while((ADC14IFGR0&0x10) == 0){}; // 7) wait for ADC14IFG4
    *x = ADC14MEM2>>4;               // 8) P6.1/A14 result 0 to 1023
    *y = ADC14MEM3>>4;               //    P4.0/A13 result 0 to 1023
    *z = ADC14MEM4>>4;               //    P4.2/A11 result 0 to 1023
}

namespace cadmium {
    struct AccelerometerInputState {
        uint16_t outputX;
        uint16_t outputY;
        uint16_t outputZ;
        uint16_t lastX;
        uint16_t lastY;
        uint16_t lastZ;
        double sigma;
        /**
         * Constructor
         */
        explicit AccelerometerInputState(): outputX(0), outputY(0), outputZ(0), lastX(0), lastY(0), lastZ(0), sigma(0) {}
    };

#ifndef NO_LOGGING
    /**
     * Insertion operator for AccelerometerInputState objects
     * @param out output stream.
     * @param s state to be represented in the output stream.
     * @return output stream
     */
    std::ostream& operator<<(std::ostream &out, const AccelerometerInputState& state) {
        out << "Accelerometer: x " << state.outputX << " y " << state.outputY << " z " << state.outputZ;
        return out;
    }
#endif

    class AccelerometerInput : public Atomic<AccelerometerInputState> {
      public:
        Port<uint16_t> outX;
        Port<uint16_t> outY;
        Port<uint16_t> outZ;

        // constructor
        AccelerometerInput(const std::string& id): Atomic<AccelerometerInputState>(id, AccelerometerInputState())  {

            outX = addOutPort<uint16_t>("outX");
            outY = addOutPort<uint16_t>("outY");
            outZ = addOutPort<uint16_t>("outZ");

            // Initialize ADC
            // Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
            ADC14CTL0 &= ~0x00000002;        // 2) ADC14ENC = 0 to allow programming
            while(ADC14CTL0&0x00010000){};   // 3) wait for BUSY to be zero
            ADC14CTL0 = 0x04223390;          // 4) single, SMCLK, on, disabled, /1, 32 SHM
            ADC14CTL1 = 0x00000030;          // 5) ADC14MEM0, 14-bit, ref on, regular power
            // 6) different for each initialization function
            ADC14IER0 = 0;
            ADC14IER1 = 0;                   // 7) no interrupts

            // Initialize Accelerometer Pins
            // Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
            ADC14MCTL2 = 0x0000000E;         // 6a) 0 to 3.3V, channel 14
            ADC14MCTL3 = 0x0000000D;         // 6b) 0 to 3.3V, channel 13
            ADC14MCTL4 = 0x0000008B;         // 6c) 0 to 3.3V, channel 11
            P6SEL0 |= 0x02;
            P6SEL1 |= 0x02;                  // 8a) analog mode on P6.1/A14
            P4SEL0 |= 0x05;
            P4SEL1 |= 0x05;                  // 8bc) analog mode on P4.2/A11 and P4.0/A13
            ADC14CTL0 |= 0x00000002;         // 9) enable


            // Get an initial reading
            BSP_Accelerometer_Input(&state.outputX, &state.outputY, &state.outputZ);

            state.lastX = state.outputX;
            state.lastY = state.outputY;
            state.lastZ = state.outputZ;
            state.sigma = 0.2;
        };

        // internal transition
        void internalTransition(AccelerometerInputState& state) const override {
            state.lastX = state.outputX;
            state.lastY = state.outputY;
            state.lastZ = state.outputZ;

            // Read pin
            BSP_Accelerometer_Input(&state.outputX, &state.outputY, &state.outputZ);
        }

        // external transition
        void externalTransition(AccelerometerInputState& state, double e) const override {
        }

        // output
        void output(const AccelerometerInputState& state) const override {
            if(state.lastX != state.outputX) {
                outX->addMessage(state.outputX);
            }
            if(state.lastY != state.outputY) {
                outY->addMessage(state.outputY);
            }
            if(state.lastZ != state.outputZ) {
                outZ->addMessage(state.outputZ);
            }
        }

        // time advance
        [[nodiscard]] double timeAdvance(const AccelerometerInputState& state) const override {
            return state.sigma;
        }
    };
} // namespace cadmium

#endif // ACCELEROMETER_INPUT_HPP
