/**
 * James Grieder & Srijan Gupta
 * ARSLab - Carleton University
 * 
 * A DEVS model for the Microphone on the Educational Boosterpack MK II
 * when used with an MSP432P401R board.
 * 
 * The microphone is polled every 0.1 seconds, and will output the reading 
 * as an integer value between 0 and 1023.
 */

#ifndef RT_MICROPHONEINPUT_HPP
#define RT_MICROPHONEINPUT_HPP

//*****************************************************************************
// ADC14 Registers
//*****************************************************************************
/* ADC Control Registers */
#define ADC14CTL0   (HWREG32(0x40012000))
#define ADC14CTL1   (HWREG32(0x40012004))
/* ADC Conversion Memory Control Registers */
#define ADC14MCTL5  (HWREG32(0x4001202C))
/* ADC Conversion Memory Registers */
#define ADC14MEM5   (HWREG32(0x400120AC))
/* Interrupt Enable Registers */
#define ADC14IER0   (HWREG32(0x4001213C))
#define ADC14IER1   (HWREG32(0x40012140))
/* Interrupt Flag Registers */
#define ADC14IFGR0  (HWREG32(0x40012144))

#include "modeling/devs/atomic.hpp"
#include <gpio.h>

#include <limits>

#ifdef RT_ARM_MBED
#endif


#ifndef NO_LOGGING
#include <iostream>
#include <fstream>
#include <string>
#endif


// ------------BSP_Microphone_Input------------
// Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
// Read and return the immediate status of the
// microphone.  The sound measurement is returned
// as a 10-bit number, even if the ADC on the
// LaunchPad is more precise.
// Input: mic is pointer to store sound measurement (0 to 1023)
// Output: none
// Assumes: BSP_Microphone_Init() has been called
void BSP_Microphone_Input(uint16_t *mic){
    ADC14CTL0 &= ~0x00000002;        // 1) ADC14ENC = 0 to allow programming
    while(ADC14CTL0&0x00010000){};   // 2) wait for BUSY to be zero
    ADC14CTL1 = (ADC14CTL1&~0x001F0000) | // clear STARTADDx bit field
            (5 << 16);           // 3) configure for STARTADDx = 5
    ADC14CTL0 |= 0x00000002;         // 4) enable conversions
    while(ADC14CTL0&0x00010000){};   // 5) wait for BUSY to be zero
    ADC14CTL0 |= 0x00000001;         // 6) start single conversion
    while((ADC14IFGR0&0x20) == 0){}; // 7) wait for ADC14IFG5
    *mic = ADC14MEM5>>4;             // 8) P4.3/A10 result 0 to 1023
}



using namespace std;

namespace cadmium {

struct MicrophoneInputState {

    uint16_t output;
    uint16_t last;
    double sigma;

    /**
     * Processor state constructor. By default, the processor is idling.
     *
     */
    explicit MicrophoneInputState(): output(0), last(0), sigma(0){
    }

};

#ifndef NO_LOGGING
/**
 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
 * @param out output stream.
 * @param s state to be represented in the output stream.
 * @return output stream with sigma already inserted.
 */
std::ostream& operator<<(std::ostream &out, const MicrophoneInputState& state) {
    out << "Pin: " << (state.output ? 1 : 0);
    return out;
}
#endif

class MicrophoneInput : public Atomic<MicrophoneInputState> {
public:

    Port<int> out;

    //Parameters to be overwritten when instantiating the atomic model
    double   pollingRate;

    // default constructor
    MicrophoneInput(const std::string& id): Atomic<MicrophoneInputState>(id, MicrophoneInputState())  {

        out = addOutPort<int>("out");

        pollingRate = 0.3;

        // Initialize ADC
        // Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
        ADC14CTL0 &= ~0x00000002;        // 2) ADC14ENC = 0 to allow programming
        while(ADC14CTL0&0x00010000){};   // 3) wait for BUSY to be zero
        ADC14CTL0 = 0x04223390;          // 4) single, SMCLK, on, disabled, /1, 32 SHM
        ADC14CTL1 = 0x00000030;          // 5) ADC14MEM0, 14-bit, ref on, regular power
        // 6) different for each initialization function
        ADC14IER0 = 0;
        ADC14IER1 = 0;                   // 7) no interrupts

        // Initialize Microphone Pin
        // Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
        ADC14MCTL5 = 0x0000008A;         // 6) 0 to 3.3V, channel 10
        P4SEL0 |= 0x08;
        P4SEL1 |= 0x08;                  // 8) analog mode on P4.3/A10
        ADC14CTL0 |= 0x00000002;         // 9) enable


        // Get an initial reading
        BSP_Microphone_Input(&state.output);

        state.last = state.output;
    };

    // internal transition
    void internalTransition(MicrophoneInputState& state) const override {
        state.last = state.output;

        // Read pin
        BSP_Microphone_Input(&state.output);

        state.sigma = pollingRate;
    }

    // external transition
    void externalTransition(MicrophoneInputState& state, double e) const override {
        throw CadmiumSimulationException("External transition called in a model with no input ports");
    }

    // output function
    void output(const MicrophoneInputState& state) const override {
        if(state.last != state.output) {
            out->addMessage((int)state.output);
        }
    }

    // time_advance function
    [[nodiscard]] double timeAdvance(const MicrophoneInputState& state) const override {
        return state.sigma;
    }
};
} 

#endif // RT_MICROPHONEINPUT_HPP
