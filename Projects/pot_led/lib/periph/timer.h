#ifndef __TIMER_H__
#define __TIMER_H__

#include <undef.h>
#include <stdint.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// Timer 1 in Arduino Uno and Arduino Mega 2560 have about 
// the same behavior and same IO addresses.
// They have DIFFERENT interruptions
//

#define TCCR1A_ADDR         0x80    // Timer/Counter1 Control Register A
#define TCCR1B_ADDR         0x81    // Timer/Counter1 Control Register B
#define OCR1A_ADDR          0x88    // Output Compare Register 1 A
#define OCR1B_ADDR          0x8a    // Output Compare Register 1 B
#define ICR1_ADDR           0x86    // Input Capture Register 1
#define TIMSK1_ADDR         0x6f    // Timer/Counter 1 Interrupt Mask Register

// Memory mapped IO addresses for Timer 1
extern volatile uint8_t * const pui8Tccr1A;   // Register TCCR1A
extern volatile uint8_t * const pui8Tccr1B;   // Register TCCR1B
extern volatile uint16_t * const pui16Ocr1A;  // Register OCR1A
extern volatile uint16_t * const pui16Ocr1B;  // Register OCR1B
extern volatile uint16_t * const pui16Icr1;    // Register ICR1
extern volatile uint8_t * const pui8Timsk1;   // Register TIMSK1

#define TCCR1A              (*pui8Tccr1A)
#define TCCR1B              (*pui8Tccr1B)
#define OCR1A               (*pui16Ocr1A)
#define OCR1B               (*pui16Ocr1B)
#define ICR1                (*pui16Icr1)
#define TIMSK1              (*pui8Timsk1)

// Timer n registers, bit definitions (timers 1, 3, 4 and 5 are identical)
#define TCCRnA_BIT_COMnA1   7
#define TCCRnA_BIT_COMnA0   6
#define TCCRnA_BIT_COMnB1   5
#define TCCRnA_BIT_COMnB0   4
#define TCCRnA_BIT_COMnC1   3
#define TCCRnA_BIT_COMnC0   2
#define TCCRnA_BIT_WGMn1    1
#define TCCRnA_BIT_WGMn0    0

#define TCCRnB_BIT_WGMn3    4
#define TCCRnB_BIT_WGMn2    3
#define TCCRnB_BIT_WGMn2    3
#define TCCRnB_DIV1         0x1
#define TCCRnB_DIV8         0x2
#define TCCRnB_DIV64        0x3
#define TCCRnB_DIV256       0x4
#define TCCRnB_DIV1024      0x5

#define TIMSKn_BIT_OCIEnC   3
#define TIMSKn_BIT_OCIEnB   2
#define TIMSKn_BIT_OCIEnA   1

#else
#error Unsupported
#endif

#endif // __TIMER_H__