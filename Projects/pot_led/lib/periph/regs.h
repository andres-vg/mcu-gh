#ifndef __REGS_H__
#define __REGS_H__

#include <undef.h>
#include <stdint.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// These regsiters in Arduino Uno and Arduino Mega 2560 have
// the same address.
//

#define SREG_ADDR           0x5f
#define PRR0_ADDR           0x64

// Memory mapped IO addresses
extern volatile uint8_t * const pui8Sreg;       // Register SREG
extern volatile uint8_t * const pui8Prr0;       // Register PRR (Uno) / PRR0 (Mega)

#define SREG (*pui8Sreg)
#define PRR0 (*pui8Prr0)

// Other control register bit definitions
#define PRR0_BIT_PRTWI      7                   // PRTWI: Power Reduction TWI

#else
#error Unsupported
#endif

#endif // __REGS_H__