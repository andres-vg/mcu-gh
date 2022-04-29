#include <regs.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// These regsiters in Arduino Uno and Arduino Mega 2560 have
// the same address.
//

// Memory mapped IO addresses
volatile uint8_t * const pui8Sreg = (uint8_t *)SREG_ADDR;       // Register SREG
volatile uint8_t * const pui8Prr0 = (uint8_t *)PRR0_ADDR;       // Register PRR/PRR0

#else
#error Unsupported
#endif
