#include "gpio.h"

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// PORT B, C, D in Arduino Uno and Arduino Mega 2560 have
// the same behavior and same IO addresses.
//

volatile uint8_t * const pui8PinB  = (uint8_t *)PINB_ADDR;      // Register PINB
volatile uint8_t * const pui8DdrB  = (uint8_t *)DDRB_ADDR;      // Register DDRB
volatile uint8_t * const pui8PortB = (uint8_t *)PORTB_ADDR;     // Register PORTB

volatile uint8_t * const pui8PinC  = (uint8_t *)PINC_ADDR;      // Register PINC
volatile uint8_t * const pui8DdrC  = (uint8_t *)DDRC_ADDR;      // Register DDRC
volatile uint8_t * const pui8PortC = (uint8_t *)PORTC_ADDR;     // Register PORTC

volatile uint8_t * const pui8PinD  = (uint8_t *)PIND_ADDR;      // Register PIND
volatile uint8_t * const pui8DdrD  = (uint8_t *)DDRD_ADDR;      // Register DDRD
volatile uint8_t * const pui8PortD = (uint8_t *)PORTD_ADDR;     // Register PORTD


#else
#error Unsupported
#endif