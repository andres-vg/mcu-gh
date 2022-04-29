#ifndef __GPIO_H__
#define __GPIO_H__

#include <undef.h>
#include <stdint.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// PORT B, C, D in Arduino Uno and Arduino Mega 2560 have 
// the same behavior and same IO addresses.
//

#define PINB_ADDR           0x23
#define DDRB_ADDR           0x24
#define PORTB_ADDR          0x25

#define PINC_ADDR           0x26
#define DDRC_ADDR           0x27
#define PORTC_ADDR          0x28

#define PIND_ADDR           0x29
#define DDRD_ADDR           0x2a
#define PORTD_ADDR          0x2b

// Memory mapped IO addresses for Port B
extern volatile uint8_t * const pui8PinB;      // Register PINB
extern volatile uint8_t * const pui8DdrB;      // Register DDRB
extern volatile uint8_t * const pui8PortB;     // Register PORTB
#define PINB (*pui8PinB)
#define DDRB (*pui8DdrB)
#define PORTB (*pui8PortB)

// Memory mapped IO addresses for Port C
extern volatile uint8_t * const pui8PinC;      // Register PINC
extern volatile uint8_t * const pui8DdrC;      // Register DDRC
extern volatile uint8_t * const pui8PortC;     // Register PORTC
#define PINC (*pui8PinC)
#define DDRC (*pui8DdrC)
#define PORTC (*pui8PortC)

// Memory mapped IO addresses for Port D
extern volatile uint8_t * const pui8PinD;      // Register PIND
extern volatile uint8_t * const pui8DdrD;      // Register DDRD
extern volatile uint8_t * const pui8PortD;     // Register PORTD
#define PIND (*pui8PinD)
#define DDRD (*pui8DdrD)
#define PORTD (*pui8PortD)

#else
#error Unsupported
#endif

#endif // __GPIO_H__