#include <timer.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// Timer 1 in Arduino Uno and Arduino Mega 2560 have about 
// the same behavior and same IO addresses.
// They have DIFFERENT interruptions
//

// Memory mapped IO addresses for Timer 1
volatile uint8_t * const pui8Tccr1A = (uint8_t *)TCCR1A_ADDR;   // Register TCCR1A
volatile uint8_t * const pui8Tccr1B = (uint8_t *)TCCR1B_ADDR;   // Register TCCR1B
volatile uint16_t * const pui16Ocr1A = (uint16_t *)OCR1A_ADDR;  // Register OCR1B
volatile uint16_t * const pui16Ocr1B = (uint16_t *)OCR1B_ADDR;  // Register OCR1B
volatile uint16_t * const pui16Icr1 = (uint16_t *)ICR1_ADDR;    // Register ICR1
volatile uint8_t * const pui8Timsk1 = (uint8_t *)TIMSK1_ADDR;   // Register TIMSK1

#else
#error Unsupported
#endif