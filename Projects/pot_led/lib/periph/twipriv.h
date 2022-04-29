#ifndef __TWI_H__
#define __TWI_H__

#include <stdint.h>

#include <undef.h>
#include <gpio.h>

typedef struct __twiContext_t
{
    uint8_t slaveAddr;
    bool twiReceiving;

    // Keep track of re-tries when sending,
    // i.e. we we don't get an ACK
    uint8_t txRetry;
    bool twiSending;
} twiContext_t;

#define TWI_MAX_TX_RETRY    1

#if defined(__AVR_ATmega328P__)

#define ISR_Twi             __vector_ ## 24

// SCL/SDA internal pull-up resistors
#define TWI_PORT            PORTC
#define TWI_DDR             DDRC
#define TWI_SCL_PULL_UP     5
#define TWI_SDA_PULL_UP     4

#elif defined(__AVR_ATmega2560__)

#define ISR_Twi             __vector_ ## 39

// SCL/SDA internal pull-up resistors
#define TWI_PORT            PORTD
#define TWI_DDR             DDRD
#define TWI_SCL_PULL_UP     0
#define TWI_SDA_PULL_UP     1

#else
#error Unsupported 
#endif


#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// TWI in Arduino Uno and Arduino Mega 2560 have 
// the same behavior and same IO addresses.
// They have DIFFERENT interruptions
//

#define TWBR_ADDR           0xb8    // TWI Bit Rate Register
#define TWSR_ADDR           0xb9    // TWI Status Register
#define TWAR_ADDR           0xba    // TWI (Slave) Address Register
#define TWDR_ADDR           0xbb    // TWI Data Register
#define TWCR_ADDR           0xbc    // TWI Control Register
#define TWAMR_ADDR          0xbd    // TWI (Slave) Address Mask Register

// Memory mapped IO addresses for TWI (I2C)
extern volatile uint8_t * const pui8Twbr; // Register TWBR
extern volatile uint8_t * const pui8Twsr; // Register TWSR
extern volatile uint8_t * const pui8Twar; // Register TWAR
extern volatile uint8_t * const pui8Twdr; // Register TWDR
extern volatile uint8_t * const pui8Twcr; // Register TWCR
extern volatile uint8_t * const pui8Twamr;// Register TWAMR

#define TWBR                (*pui8Twbr)
#define TWSR                (*pui8Twsr)
#define TWAR                (*pui8Twar)
#define TWDR                (*pui8Twdr)
#define TWCR                (*pui8Twcr)
#define TWAMR               (*pui8Twamr)

// TWI Control register bit definitions
#define TWCR_BIT_TWINT      7       // TWI Interrupt Flag
#define TWCR_BIT_TWEA       6       // TWI Enable Acknowledge Bit
#define TWCR_BIT_TWSTA      5       // TWI START Condition Bit
#define TWCR_BIT_TWSTO      4       // TWI STOP Condition Bit
#define TWCR_BIT_TWWC       3       // TWI Write Collision Flag
#define TWCR_BIT_TWEN       2       // TWI Enable Bit
#define TWCR_BIT_TWIE       0       // TWI Interrupt Enable

#define TWCR_MASK_READY     (b2m(TWCR_BIT_TWEN) | b2m(TWCR_BIT_TWIE) | \
                             b2m(TWCR_BIT_TWEA))

// TWI (Slave) Address Register bit definitions
// Bits 7:6 define the slave address
#define TWAR_BIT_TWGCE      0       // TWI General Call Recognition Enable Bit

// SLA+R/W (TWDR) definitions (1:Rd, 0:Wr)
#define SLA_RW_BIT_RD       0

// Set the value of TPWS1:0 and TWBR register for the given SCL freq.
inline void twiSetBitRateDivisionFactor(
        const unsigned long SCLFreqKHz,     // Target SCL frequency
        const unsigned char twps            // TWPS
)
{
    // This is the devisor as defined in data sheet
    // "Bit Rate Generator Unit"
    //
    // SCL frequency = CPU clock frequency / { 16 + [ 2 * TWBR * 4^TWPS ] }
    // or
    // TWBR = { [ CPU clock frequency / SCL frequency] - 16 } / { 2 * 4^TPWS }
    // simplified to (using SCL frequency in KHz):
    // TWBR = { [ ( CPU clock frequency / 2000 ) / SCL frequency KHz ] - 8 } / { 4 ^ TPWS }
    //
    // 4 ^ TWPS implemented with left shift operations to obtain
    // a pre-scaler of 1, 4, 16 or 64 for TWPS 0, 1, 2, 3 respectively.
    //
    // These are ranges for all TWPS values:
    // TWPS = 0: SCL freqs. [31, 888] KHz, Returned values [250, 1]
    // TWPS = 1: SCL freqs. [ 8, 666] KHz, Returned values [248, 1]
    // TWPS = 2: SCL freqs. [ 2, 333] KHz, Returned values [250, 1]
    // TWPS = 3: SCL freqs. [ .5,111] KHz, Returned values [250, 1]
    TWSR = twps & 0x3;
    TWBR = (((F_CPU / 2000) / SCLFreqKHz) - 8) / (1 << ((twps & 0x3) << 1));
}

#else
#error Unsupported
#endif

#endif // __TWI_H__