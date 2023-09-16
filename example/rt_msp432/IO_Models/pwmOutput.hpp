/**
 * James Grieder & Srijan Gupta
 * ARSLab - Carleton University
 * 
 * A DEVS model for pulse-width modulation outputs (pwm) on the Educational 
 * Boosterpack MK II when used with an MSP432P401R board.  This model can be 
 * used for the individual RGB LEDs, and the buzzer output.
 * 
 * Upon receiving an integer value in the external transition function, the duty 
 * cycle of the associated pwm output pin is updated.  The duty cycle must be 
 * between 0 and 1023, inclusively.
 */

#ifndef __MSP_PWMOUTPUT_HPP__
#define __MSP_PWMOUTPUT_HPP__

#include "modeling/devs/atomic.hpp"
#include <gpio.h>
#include <rom.h>
#include <limits>

#ifndef NO_LOGGING
    #include <iostream>
    #include <string>
    #include <fstream>
#endif

using namespace std;

namespace cadmium {

struct PWMOutputState {
    int output;
    double sigma;

    /**
     * Processor state constructor. By default, the processor is idling.
     */
    explicit PWMOutputState(): output(0), sigma(0){
    }

};

#ifndef NO_LOGGING
/**
 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
 * @param out output stream.
 * @param s state to be represented in the output stream.
 * @return output stream with sigma already inserted.
 */

std::ostream& operator<<(std::ostream &out, const PWMOutputState& state) {
    out << "Pin: " << (state.output ? 1 : 0);
    return out;
}
#endif

class PWMOutput : public Atomic<PWMOutputState> {
public:

    Port<int> in;
    //Parameters to be overwritten when instantiating the atomic model
    uint_fast8_t port;
    uint_fast16_t pin;



    // default constructor
    PWMOutput(const std::string& id, uint_fast8_t selectedPort,uint_fast16_t selectedPin): Atomic<PWMOutputState>(id, PWMOutputState())  { 
        in = addInPort<int>("in");
        port = selectedPort;
        pin = selectedPin;

        uint16_t initialDutyCycle = 0;

        // Constants for PWM calculations
        uint32_t SubsystemFrequency = 3000000; // 3MHz
        uint16_t PWMCycles = SubsystemFrequency/2048;

        if (port == GPIO_PORT_P2) {
            // Configure Timer0 for PWM
            TA0CTL &= ~0x0030;              // halt Timer
            TA0CTL = 0x0200;                // set clock source to SMCLK

            // Pin specific configs
            if (pin == GPIO_PIN4) { // RGB Green
                TA0CCTL1 = 0x00E0;                          // OUTMOD = reset/set
                TA0CCR1 = (initialDutyCycle*PWMCycles)>>10; // defines when output signal is cleared

            } else if (pin == GPIO_PIN6) { // RGB Red
                TA0CCTL3 = 0x00E0;                          // OUTMOD = reset/set
                TA0CCR3 = (initialDutyCycle*PWMCycles)>>10; // defines when output signal is cleared

            } else if (pin == GPIO_PIN7) { // Buzzer
                TA0CCTL4 = 0x00E0;                          // OUTMOD = reset/set
                TA0CCR4 = (initialDutyCycle*PWMCycles)>>10; // defines when output signal is cleared

            } else {
                throw CadmiumSimulationException("incorrect port/pin combination for pwmOutput");
            }

            // Configure Timer0 for PWM
            TA0CCR0 = PWMCycles - 1;         // defines when output signal is set
            TA0EX0 &= ~0x0007;               // configure for input clock divider /1
            TA0CTL |= 0x0014;                // reset and start Timer A0 in up mode

            // Pin specific configs
            P2SEL0 |= pin;
            P2SEL1 &= ~pin;                 // configure P2.4 as timer out
            P2DIR |= pin;                   // make P2.4 out


        } else if (port == GPIO_PORT_P5) {

            if (pin == GPIO_PIN6) {
                // Configure TimerA2 for PWM
                TA2CTL &= ~0x0030;                      // halt Timer
                TA2CTL = 0x0200;                        // set clock source to SMCLK
                TA2CCTL1 = 0x00E0;                      // OUTMOD = reset/set
                TA2CCR1 = (initialDutyCycle*1464)>>10;  // defines when output signal is cleared
                TA2CCR0 = PWMCycles - 1;                // defines when output signal is set
                TA2EX0 &= ~0x0007;                      // configure for input clock divider /1
                TA2CTL |= 0x0014;                       // start timer in UP mode

                // Pin specific configs
                P5SEL0 |= pin;
                P5SEL1 &= ~pin;                 // configure P5.6 as timer out
                P5DIR |= pin;                   // make P5.6 out
            } else {
                throw CadmiumSimulationException("incorrect port/pin combination for pwmOutput");
            }

        } else {
            throw CadmiumSimulationException("incorrect port/pin combination for pwmOutput");
        }

    };

    // internal transition
    void internalTransition(PWMOutputState& state) const override {
    }

    // external transition
    void externalTransition(PWMOutputState& state, double e) const override {
        if(!in->empty()){
            for( const auto x : in->getBag()){
                state.output = x;

                if (x < 0) {
                    state.output = 0;
                } else if (x > 1023) {
                    state.output = 1023;
                } else { 
                    state.output = x;
                }
            }

            if (port == GPIO_PORT_P2) {
                if (pin == GPIO_PIN4) { // RGB Green
                    TA0CCR1 = ((uint16_t)(state.output*1464)>>10);

                } else if (pin == GPIO_PIN6) { // RGB Red
                    TA0CCR3 = ((uint16_t)(state.output*1464)>>10);

                } else if (pin == GPIO_PIN7) { // Buzzer
                    TA0CCR4 = ((uint16_t)(state.output*1464)>>10);

                }

            } else if (port == GPIO_PORT_P5) {
                if (pin == GPIO_PIN6) {
                    TA2CCR1 = ((uint16_t)(state.output*1464)>>10);

                }
            }
        }
    }


    // output function
    void output(const PWMOutputState& state) const override {
    };

    // time_advance function
    [[nodiscard]] double timeAdvance(const PWMOutputState& state) const override {
        return std::numeric_limits<double>::infinity();
    }

};
}

#endif // __MSP_PWMOUTPUT_HPP__
