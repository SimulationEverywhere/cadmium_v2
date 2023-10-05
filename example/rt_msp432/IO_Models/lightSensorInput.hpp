/**
 * James Grieder & Srijan Gupta
 * ARSLab - Carleton University
 * 
 * A DEVS model for the Light Sensor on the Educational Boosterpack MK II
 * when used with an MSP432P401R board.
 * 
 * The light sensor is polled every 0.8 seconds (based on the duration of 
 * a single reading), and will output the reading as an integer value 
 * measured in units of lux.
 */

#ifndef RT_LIGHTSENSORINPUT_HPP
#define RT_LIGHTSENSORINPUT_HPP

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



// There are two I2C devices on the Educational BoosterPack MKII:
// OPT3001 Light Sensor
// TMP006 Temperature sensor
// Both initialization functions can use this general I2C
// initialization.
void static i2cinit(void){
  // initialize eUSCI
  UCB1CTLW0 = 0x0001;                // hold the eUSCI module in reset mode
  // configure UCB1CTLW0 for:
  // bit15      UCA10 = 0; own address is 7-bit address
  // bit14      UCSLA10 = 0; address slave with 7-bit address
  // bit13      UCMM = 0; single master environment
  // bit12      reserved
  // bit11      UCMST = 1; master mode
  // bits10-9   UCMODEx = 3; I2C mode
  // bit8       UCSYNC = 1; synchronous mode
  // bits7-6    UCSSELx = 2; eUSCI clock SMCLK
  // bit5       UCTXACK = X; transmit ACK condition in slave mode
  // bit4       UCTR = X; transmitter/receiver
  // bit3       UCTXNACK = X; transmit negative acknowledge in slave mode
  // bit2       UCTXSTP = X; transmit stop condition in master mode
  // bit1       UCTXSTT = X; transmit start condition in master mode
  // bit0       UCSWRST = 1; reset enabled
  UCB1CTLW0 = 0x0F81;
  // configure UCB1CTLW1 for:
  // bits15-9   reserved
  // bit8       UCETXINT = X; early UCTXIFG0 in slave mode
  // bits7-6    UCCLTO = 3; timeout clock low after 165,000 SYSCLK cycles
  // bit5       UCSTPNACK = 0; send negative acknowledge before stop condition in master receiver mode
  // bit4       UCSWACK = 0; slave address acknowledge controlled by hardware
  // bits3-2    UCASTPx = 2; generate stop condition automatically after UCB0TBCNT bytes
  // bits1-0    UCGLITx = 0 deglitch time of 50 ns
  UCB1CTLW1 = 0x00C8;
  UCB1TBCNT = 2;                     // generate stop condition after this many bytes
  // set the baud rate for the eUSCI which gets its clock from SMCLK
  // Clock_Init48MHz() from ClockSystem.c sets SMCLK = HFXTCLK/4 = 12 MHz
  // if the SMCLK is set to 12 MHz, divide by 120 for 100 kHz baud clock
  UCB1BRW = 120;
  P6SEL0 |= 0x30;
  P6SEL1 &= ~0x30;                   // configure P6.5 and P6.4 as primary module function
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1IE = 0x0000;                   // disable interrupts
}

/*// receives one byte from specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
// Note for TMP102 thermometer only:
// Used to read the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
uint8_t static I2C_Recv(int8_t slave){
  int8_t data1;
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 1;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0014)   // clear bit4 (UCTR) for receive mode
                                     // clear bit2 (UCTXSTP) for no transmit stop condition
                | 0x0002);           // set bit1 (UCTXSTT) for transmit start condition
  while((UCB1IFG&0x0001) == 0){      // wait for complete character received
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      i2cinit();                     // reset to known state
      return 0xFF;
    }
  }
  data1 = UCB1RXBUF&0xFF;            // get the reply
  return data1;
}*/

// receives two bytes from specified slave
// Note for HMC6352 compass only:
// Used with 'A' commands
// Note for TMP102 thermometer only:
// Used to read the contents of the pointer register
uint16_t static I2C_Recv2(int8_t slave){
  uint8_t data1, data2;
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 2;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0014)   // clear bit4 (UCTR) for receive mode
                                     // clear bit2 (UCTXSTP) for no transmit stop condition
                | 0x0002);           // set bit1 (UCTXSTT) for transmit start condition
  while((UCB1IFG&0x0001) == 0){      // wait for complete character received
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      i2cinit();                     // reset to known state
      return 0xFFFF;
    }
  }
  data1 = UCB1RXBUF&0xFF;            // get the reply
  while((UCB1IFG&0x0001) == 0){      // wait for complete character received
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      i2cinit();                     // reset to known state
      return 0xFFFF;
    }
  }
  data2 = UCB1RXBUF&0xFF;            // get the reply
  return (data1<<8)+data2;
}

// sends one byte to specified slave
// Note for HMC6352 compass only:
// Used with 'S', 'W', 'O', 'C', 'E', 'L', and 'A' commands
//  For 'A' commands, I2C_Recv2() should also be called
// Note for TMP102 thermometer only:
// Used to change the pointer register
// Returns 0 if successful, nonzero if error
uint16_t static I2C_Send1(int8_t slave, uint8_t data1){
  uint16_t debugdump;                // save status register here in case of error
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 1;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while(UCB1CTLW0&0x0002){};         // wait for slave address sent
  UCB1TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while(UCB1STATW&0x0010){           // wait for I2C idle
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      i2cinit();                     // reset to known state
      return debugdump;
    }
  }
  return 0;
}

/*// sends two bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
//  For 'r' and 'g' commands, I2C_Recv() should also be called
// Note for TMP102 thermometer only:
// Used to change the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
// Returns 0 if successful, nonzero if error
uint16_t static I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2){
  uint16_t debugdump;                // save status register here in case of error
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 2;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while(UCB1CTLW0&0x0002){};         // wait for slave address sent
  UCB1TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while((UCB1IFG&0x0002) == 0){      // wait for first data sent
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      i2cinit();                     // reset to known state
      return debugdump;
    }
  }
  UCB1TXBUF = data2&0xFF;            // TXBUF[7:0] is data
  while(UCB1STATW&0x0010){           // wait for I2C idle
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      i2cinit();                     // reset to known state
      return debugdump;
    }
  }
  return 0;
}*/

// sends three bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'w' and 'G' commands
// Note for TMP102 thermometer only:
// Used to change the contents of the pointer register
// Returns 0 if successful, nonzero if error
uint16_t static I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3){
  uint16_t debugdump;                // save status register here in case of error
  while(UCB1STATW&0x0010){};         // wait for I2C ready
  UCB1CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB1TBCNT = 3;                     // generate stop condition after this many bytes
  UCB1CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB1I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB1CTLW0 = ((UCB1CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while((UCB1IFG&0x0002) == 0){};    // wait for slave address sent
  UCB1TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while((UCB1IFG&0x0002) == 0){      // wait for first data sent
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB0IFG;           // snapshot flag register for calling program
      i2cinit();                     // reset to known state
      return debugdump;
    }
  }
  UCB1TXBUF = data2&0xFF;            // TXBUF[7:0] is data
  while((UCB1IFG&0x0002) == 0){      // wait for second data sent
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      i2cinit();                     // reset to known state
      return debugdump;
    }
  }
  UCB1TXBUF = data3&0xFF;            // TXBUF[7:0] is data
  while(UCB1STATW&0x0010){           // wait for I2C idle
    if(UCB1IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      debugdump = UCB1IFG;           // snapshot flag register for calling program
      i2cinit();                     // reset to known state
      return debugdump;
    }
  }
  return 0;
}

// ------------BSP_LightSensor_Init------------
// Initialize a GPIO pin for input, which corresponds
// with BoosterPack pins J1.8 (Light Sensor interrupt).
// Initialize two I2C pins, which correspond with
// BoosterPack pins J1.9 (SCL) and J1.10 (SDA).
// Input: none
// Output: none
void BSP_LightSensor_Init(void){
  i2cinit();
  P4SEL0 &= ~0x40;
  P4SEL1 &= ~0x40;                 // configure P4.6 as GPIO
  P4DIR &= ~0x40;                  // make P4.6 in
  P4REN &= ~0x40;                  // disable pull resistor on P4.6
}

// Send the appropriate codes to initiate a
// measurement with an OPT3001 light sensor at I2C
// slave address 'slaveAddress'.
// Assumes: BSP_LightSensor_Init() has been called
void static lightsensorstart(uint8_t slaveAddress){
  // configure Low Limit Register (0x02) for:
  // INT pin active after each conversion completes
  I2C_Send3(slaveAddress, 0x02, 0xC0, 0x00);
  // configure Configuration Register (0x01) for:
  // 15-12 RN         range number         1100b = automatic full-scale setting mode
  // 11    CT         conversion time         1b = 800 ms
  // 10-9  M          mode of conversion     01b = single-shot
  // 8     OVF        overflow flag field     0b (read only)
  // 7     CRF        conversion ready field  0b (read only)
  // 6     FH         flag high field         0b (read only)
  // 5     FL         flag low field          0b (read only)
  // 4     L          latch                   1b = latch interrupt if measurement exceeds programmed ranges
  // 3     POL        polarity                0b = INT pin reports active low
  // 2     ME         mask exponent           0b = do not mask exponent (more math later)
  // 1-0   FC         fault count            00b = 1 fault triggers interrupt
  I2C_Send3(slaveAddress, 0x01, 0xCA, 0x10);
  I2C_Recv2(slaveAddress);         // read Configuration Register to reset conversion ready
}

// Send the appropriate codes to end a measurement
// with an OPT3001 light sensor at I2C slave address
// 'slaveAddress'.  Return results (units 100*lux).
// Assumes: BSP_LightSensor_Init() has been called and measurement is ready
int32_t static lightsensorend(uint8_t slaveAddress){
  uint16_t raw, config;
  I2C_Send1(slaveAddress, 0x00);   // pointer register 0x00 = Result Register
  raw = I2C_Recv2(slaveAddress);
  // force the INT pin to clear by clearing and resetting the latch bit of the Configuration Register (0x01)
  I2C_Send1(slaveAddress, 0x01);   // pointer register 0x01 = Configuration Register
  config = I2C_Recv2(slaveAddress);// current Configuration Register
  I2C_Send3(slaveAddress, 0x01, (config&0xFF00)>>8, (config&0x00FF)&~0x0010);
  I2C_Send3(slaveAddress, 0x01, (config&0xFF00)>>8, (config&0x00FF)|0x0010);
  return (1<<(raw>>12))*(raw&0x0FFF);
}

// ------------BSP_LightSensor_Input------------
// Query the OPT3001 light sensor for a measurement.
// Wait until the measurement is ready, which may
// take 800 ms.
// Input: none
// Output: light intensity (units 100*lux)
// Assumes: BSP_LightSensor_Init() has been called
#define LIGHTINT  (*((volatile uint8_t *)(0x42000000+32*0x4C21+4*6)))  /* Port 4.6 Input */
int LightBusy = 0;                 // 0 = idle; 1 = measuring
uint32_t BSP_LightSensor_Input(void){
  uint32_t light;
  LightBusy = 1;
  lightsensorstart(0x44);
  while(LIGHTINT == 0x01){};       // wait for conversion to complete
  light = lightsensorend(0x44);
  LightBusy = 0;
  return light;
}

// ------------BSP_LightSensor_Start------------
// Start a measurement using the OPT3001.
// If a measurement is currently in progress, return
// immediately.
// Input: none
// Output: none
// Assumes: BSP_LightSensor_Init() has been called
void BSP_LightSensor_Start(void){
  if(LightBusy == 0){
    // no measurement is in progress, so start one
    LightBusy = 1;
    lightsensorstart(0x44);
  }
}

// ------------BSP_LightSensor_End------------
// Query the OPT3001 light sensor for a measurement.
// If no measurement is currently in progress, start
// one and return zero immediately.  If the measurement
// is not yet complete, return zero immediately.  If
// the measurement is complete, store the result in the
// pointer provided and return one.
// Input: light is pointer to store light intensity (units 100*lux)
// Output: one if measurement is ready and pointer is valid
//         zero if measurement is not ready and pointer unchanged
// Assumes: BSP_LightSensor_Init() has been called
int BSP_LightSensor_End(uint32_t *light){
  uint32_t lightLocal;
  if(LightBusy == 0){
    // no measurement is in progress, so start one
    LightBusy = 1;
    lightsensorstart(0x44);
    return 0;                      // measurement needs more time to complete
  } else{
    // measurement is in progress
    if(LIGHTINT == 0x01){
      return 0;                    // measurement needs more time to complete
    } else{
      lightLocal = lightsensorend(0x44);
      *light = lightLocal;
      LightBusy = 0;
      return 1;                    // measurement is complete; pointer valid
    }
  }
}



using namespace std;

namespace cadmium {

struct LightSensorInputState {

    uint32_t output;
    uint32_t last;
    double sigma;

    /**
     * Processor state constructor. By default, the processor is idling.
     *
     */
    explicit LightSensorInputState(): output(0), last(0), sigma(0){
    }

};

#ifndef NO_LOGGING
/**
 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
 * @param out output stream.
 * @param s state to be represented in the output stream.
 * @return output stream with sigma already inserted.
 */
std::ostream& operator<<(std::ostream &out, const LightSensorInputState& state) {
    out << "Pin: " << (state.output ? 1 : 0);
    return out;
}
#endif



class LightSensorInput : public Atomic<LightSensorInputState> {
public:

    Port<int> out;

    //Parameters to be overwritten when instantiating the atomic model
    double   pollingRate;

    // default constructor
    LightSensorInput(const std::string& id): Atomic<LightSensorInputState>(id, LightSensorInputState())  {

        out = addOutPort<int>("out");

        pollingRate = 0.8;

        // Initialize
        BSP_LightSensor_Init();

        // Get an initial reading
        state.output = BSP_LightSensor_Input() / 100; // Division by 100 converts the value to lux

        state.last = state.output;
    };

    // internal transition
    void internalTransition(LightSensorInputState& state) const override {
        state.last = state.output;

        // Read pin
        state.output = BSP_LightSensor_Input() / 100; // Division by 100 converts the value to lux

        state.sigma = pollingRate;
    }

    // external transition
    void externalTransition(LightSensorInputState& state, double e) const override {
        throw CadmiumSimulationException("External transition called in a model with no input ports");
    }

    // output function
    void output(const LightSensorInputState& state) const override {
        if(state.last != state.output) {
            out->addMessage((int)state.output);
        }
    }

    // time_advance function
    [[nodiscard]] double timeAdvance(const LightSensorInputState& state) const override {
        return state.sigma;
    }
};
} 

#endif // RT_LIGHTSENSORINPUT_HPP
