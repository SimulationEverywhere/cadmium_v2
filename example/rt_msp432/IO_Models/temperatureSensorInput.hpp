/**
 * James Grieder & Srijan Gupta
 * ARSLab - Carleton University
 * 
 * A DEVS model for the Temperature Sensor on the Educational Boosterpack MK II
 * when used with an MSP432P401R board.
 * 
 * The temperature sensor is polled every 4.0 seconds (based on the duration of 
 * a single reading), and will output the reading as an integer value 
 * measured in units of degrees Celsius, multiplied by a factor of 100,000.
 */

#ifndef RT_TEMPERATURESENSORINPUT_HPP
#define RT_TEMPERATURESENSORINPUT_HPP

#include "modeling/devs/atomic.hpp"
#include <gpio.h>

#include <limits>


#ifndef NO_LOGGING
    #include <iostream>
    #include <fstream>
    #include <string>
#endif




using namespace std;

namespace cadmium {

struct TemperatureSensorInputState {

    int32_t output;
    int32_t last;
    int32_t sensorV;
    double sigma;

    /**
     * Processor state constructor. By default, the processor is idling.
     *
     */
    explicit TemperatureSensorInputState(): output(0), last(0), sensorV(0), sigma(0){
    }

};

#ifndef NO_LOGGING
/**
 * Insertion operator for ProcessorState objects. It only displays the value of sigma.
 * @param out output stream.
 * @param s state to be represented in the output stream.
 * @return output stream with sigma already inserted.
 */
std::ostream& operator<<(std::ostream &out, const TemperatureSensorInputState& state) {
    out << "Pin: " << (state.output ? 1 : 0);
    return out;
}
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

// ------------BSP_TempSensor_Init------------
// Initialize a GPIO pin for input, which corresponds
// with BoosterPack pins J2.11 (Temperature Sensor
// interrupt).  Initialize two I2C pins, which
// correspond with BoosterPack pins J1.9 (SCL) and
// J1.10 (SDA).
// Input: none
// Output: none
void BSP_TempSensor_Init(void){
  i2cinit();
  P3SEL0 &= ~0x40;
  P3SEL1 &= ~0x40;                 // configure P3.6 as GPIO
  P3DIR &= ~0x40;                  // make P3.6 in
  P3REN &= ~0x40;                  // disable pull resistor on P3.6
}

// Send the appropriate codes to initiate a
// measurement with a TMP006 temperature sensor at
// I2C slave address 'slaveAddress'.
// Assumes: BSP_TempSensor_Init() has been called
void static tempsensorstart(uint8_t slaveAddress){
  // configure Configuration Register (0x02) for:
  // 15    RST        software reset bit      0b = normal operation
  // 14-12 MOD        mode of operation     111b = sensor and die continuous conversion
  // 11-9  CR         ADC conversion rate   010b = 1 sample/sec
  // 8     EN         interrupt pin enable    1b = ~DRDY pin enabled (J2.11/P3.6)
  // 7     ~DRDY      data ready bit          0b (read only, automatic clear)
  // 6-0   reserved                      000000b (reserved)
  I2C_Send3(slaveAddress, 0x02, 0x75, 0x00);
}

// Send the appropriate codes to end a measurement
// with a TMP006 temperature sensor at I2C slave
// address 'slaveAddress'.  Store the results at the
// provided pointers.
// Assumes: BSP_TempSensor_Init() has been called and measurement is ready
void static tempsensorend(uint8_t slaveAddress, int32_t *sensorV, int32_t *localT){
  int16_t raw;
  I2C_Send1(slaveAddress, 0x00);   // pointer register 0x00 = Sensor Voltage Register
  raw = I2C_Recv2(slaveAddress);
  *sensorV = raw*15625;            // 156.25 nV per LSB
  I2C_Send1(slaveAddress, 0x01);   // pointer register 0x01 = Local Temperature Register
  raw = I2C_Recv2(slaveAddress);
  *localT = (raw>>2)*3125;         // 0.03125 C per LSB
}

// ------------BSP_TempSensor_Input------------
// Query the TMP006 temperature sensor for a
// measurement.  Wait until the measurement is ready,
// which may take 4 seconds.
// Input: sensorV is signed pointer to store sensor voltage (units 100*nV)
//        localT is signed pointer to store local temperature (units 100,000*C)
// Output: none
// Assumes: BSP_TempSensor_Init() has been called
#define TEMPINT   (*((volatile uint8_t *)(0x42000000+32*0x4C20+4*6)))  /* Port 3.6 Input */
int TempBusy = 0;                  // 0 = idle; 1 = measuring
void BSP_TempSensor_Input(int32_t *sensorV, int32_t *localT){
  int32_t volt, temp;
  TempBusy = 1;
  tempsensorstart(0x40);
  while(TEMPINT == 0x01){};        // wait for conversion to complete
  tempsensorend(0x40, &volt, &temp);
  *sensorV = volt;
  *localT = temp;
  TempBusy = 0;
}

// ------------BSP_TempSensor_Start------------
// Start a measurement using the TMP006.
// If a measurement is currently in progress, return
// immediately.
// Input: none
// Output: none
// Assumes: BSP_TempSensor_Init() has been called
void BSP_TempSensor_Start(void){
  if(TempBusy == 0){
    // no measurement is in progress, so start one
    TempBusy = 1;
    tempsensorstart(0x40);
  }
}

// ------------BSP_TempSensor_End------------
// Query the TMP006 temperature sensor for a
// measurement.  If no measurement is currently in
// progress, start one and return zero immediately.
// If the measurement is not yet complete, return
// zero immediately.  If the measurement is complete,
// store the result in the pointers provided and
// return one.
// Input: sensorV is signed pointer to store sensor voltage (units 100*nV)
//        localT is signed pointer to store local temperature (units 100,000*C)
// Output: one if measurement is ready and pointers are valid
//         zero if measurement is not ready and pointers unchanged
// Assumes: BSP_TempSensor_Init() has been called
int BSP_TempSensor_End(int32_t *sensorV, int32_t *localT){
  int32_t volt, temp;
  if(TempBusy == 0){
    // no measurement is in progress, so start one
    TempBusy = 1;
    tempsensorstart(0x40);
    return 0;                      // measurement needs more time to complete
  } else{
    // measurement is in progress
    if(TEMPINT == 0x01){
      return 0;                    // measurement needs more time to complete
    } else{
      tempsensorend(0x40, &volt, &temp);
      *sensorV = volt;
      *localT = temp;
      TempBusy = 0;
      return 1;                    // measurement is complete; pointers valid
    }
  }
}



class TemperatureSensorInput : public Atomic<TemperatureSensorInputState> {
public:

    Port<double> out;

    //Parameters to be overwritten when instantiating the atomic model
    double   pollingRate;

    // default constructor
    TemperatureSensorInput(const std::string& id): Atomic<TemperatureSensorInputState>(id, TemperatureSensorInputState())  {

        out = addOutPort<double>("out");

        pollingRate = 4.0;

        // Initialize
        BSP_TempSensor_Init();

        // Get an initial reading
        BSP_TempSensor_Input(&state.sensorV, &state.output);

        state.last = state.output;
    };

    // internal transition
    void internalTransition(TemperatureSensorInputState& state) const override {
        state.last = state.output;

        // Read pin
        BSP_TempSensor_Input(&state.sensorV, &state.output);

        state.sigma = pollingRate;
    }

    // external transition
    void externalTransition(TemperatureSensorInputState& state, double e) const override {
        throw CadmiumSimulationException("External transition called in a model with no input ports");
    }

    // output function
    void output(const TemperatureSensorInputState& state) const override {
        out->addMessage((double)state.output);
    }

    // time_advance function
    [[nodiscard]] double timeAdvance(const TemperatureSensorInputState& state) const override {
        return state.sigma;
    }
};
} 

#endif // RT_TEMPERATURESENSORINPUT_HPP
