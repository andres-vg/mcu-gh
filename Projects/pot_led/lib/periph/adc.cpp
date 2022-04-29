#include <adc.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// ADC in Arduino Uno and Arduino Mega 2560 have about 
// the same behavior and same IO addresses.
// They have DIFFERENT interruptions.
//

// Memory mapped IO addresses for ADC
volatile uint16_t * const pui16Adc  = (uint16_t *)ADC_ADDR;
volatile uint8_t * const pui8AdcH  = (uint8_t *)ADCH_ADDR;
volatile uint8_t * const pui8AdcL  = (uint8_t *)ADCL_ADDR;
volatile uint8_t * const pui8Adcsra  = (uint8_t *)ADCSRA_ADDR;
volatile uint8_t * const pui8Adcsrb  = (uint8_t *)ADCSRB_ADDR;
volatile uint8_t * const pui8Admux  = (uint8_t *)ADMUX_ADDR;
#if defined(__AVR_ATmega2560__)
volatile uint8_t * const pui8Didr2  = (uint8_t *)DIDR2_ADDR;
#endif
volatile uint8_t * const pui8Didr0  = (uint8_t *)DIDR0_ADDR;

#else
#error Unsupported
#endif