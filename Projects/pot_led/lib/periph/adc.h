#ifndef __ADC_H__
#define __ADC_H__

#include <undef.h>
#include <stdint.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)

//
// NOTE
//
// ADC in Arduino Uno and Arduino Mega 2560 have about 
// the same behavior and same IO addresses.
// They have DIFFERENT interruptions.
//

#define ADC_ADDR            0x78    // The ADC Data Register
#define ADCH_ADDR           0x79    // The ADC Data Register
#define ADCL_ADDR           0x78    // The ADC Data Register
#define ADCSRA_ADDR         0x7a    // ADC Control and Status Register A
#define ADCSRB_ADDR         0x7b    // ADC Control and Status Register B
#define ADMUX_ADDR          0x7c    // ADC Multiplexer Selection Register
#if defined(__AVR_ATmega2560__)
#define DIDR2_ADDR          0x7d    // Digital Input Disable Register 2, NOTE There is no DIDR1
#endif
#define DIDR0_ADDR          0x7e    // Digital Input Disable Register 0, NOTE There is no DIDR1

// Memory mapped IO addresses for ADC
extern volatile uint16_t * const pui16Adc;
extern volatile uint8_t * const pui8AdcH;
extern volatile uint8_t * const pui8AdcL;
extern volatile uint8_t * const pui8Adcsra;
extern volatile uint8_t * const pui8Adcsrb;
extern volatile uint8_t * const pui8Admux;
#if defined(__AVR_ATmega2560__)
extern volatile uint8_t * const pui8Didr2;
#endif
extern volatile uint8_t * const pui8Didr0;
#define ADC                 (*pui16Adc)
#define ADCH                (*pui8AdcH)
#define ADCL                (*pui8AdcL)
#define ADCSRA              (*pui8Adcsra)
#define ADCSRB              (*pui8Adcsrb)
#define ADMUX               (*pui8Admux)
#define DIDR2               (*pui8Didr2)
#define DIDR0               (*pui8Didr0)

// ADCSRA bit definitions
#define ADCSRA_BIT_ADEN     7       // ADC Enable
#define ADCSRA_BIT_ADSC     6       // ADC Start Conversion
#define ADCSRA_BIT_ADATE    5       // ADC Auto Trigger Enable
#define ADCSRA_BIT_ADIF     4       // ADC Interrupt Flag
#define ADCSRA_BIT_ADIE     3       // ADC Interrupt Enable
#define ADCSRA_BIT_ADPS2    2       // ADC Prescaler Selections bit 2
#define ADCSRA_BIT_ADPS1    1       // ADC Prescaler Selections bit 1
#define ADCSRA_BIT_ADPS0    0       // ADC Prescaler Selections bit 0
#define ADCSRA_DIV2         0x1     // Prescaler, divide by 2
#define ADCSRA_DIV4         0x2     // Prescaler, divide by 4
#define ADCSRA_DIV8         0x3     // Prescaler, divide by 8
#define ADCSRA_DIV16        0x4     // Prescaler, divide by 16
#define ADCSRA_DIV32        0x5     // Prescaler, divide by 32
#define ADCSRA_DIV64        0x6     // Prescaler, divide by 64
#define ADCSRA_DIV128       0x7     // Prescaler, divide by 128

// ADCSRB bit definitions
#define ADCSRB_BIT_MUX5     3       // MUX5: Analog Channel and Gain Selection Bit
#define ADCSRB_BIT_ADTS2    2       // ADTS2 ADC Auto Trigger Source bit 2
#define ADCSRB_BIT_ADTS1    1       // ADTS1 ADC Auto Trigger Source bit 1
#define ADCSRB_BIT_ADTS0    0       // ADTS0 ADC Auto Trigger Source bit 0

// ADMUX bit definitions
#define ADMUX_BIT_REFS1     7       // REFS1 Reference Selection bit 1
#define ADMUX_BIT_REFS0     6       // REFS0 Reference Selection bit 0
#define ADMUX_BIT_ADLAR     5       // ADLAR: ADC Left Adjust Result
#define ADMUX_BIT_MUX4      4       // MUX4: Analog Channel and Gain Selection bit 4
#define ADMUX_BIT_MUX3      3       // MUX4: Analog Channel and Gain Selection bit 3
#define ADMUX_BIT_MUX2      2       // MUX4: Analog Channel and Gain Selection bit 2
#define ADMUX_BIT_MUX1      1       // MUX4: Analog Channel and Gain Selection bit 1
#define ADMUX_BIT_MUX0      0       // MUX4: Analog Channel and Gain Selection bit 0

// DIDR2 bit definitions
#define DIDR2_BIT_ADC15D    7       // ADC15 Digital Input Disable
#define DIDR2_BIT_ADC14D    6       // ADC14 Digital Input Disable
#define DIDR2_BIT_ADC13D    5       // ADC13 Digital Input Disable
#define DIDR2_BIT_ADC12D    4       // ADC12 Digital Input Disable
#define DIDR2_BIT_ADC11D    3       // ADC11 Digital Input Disable
#define DIDR2_BIT_ADC10D    2       // ADC10 Digital Input Disable
#define DIDR2_BIT_ADC9D     1       // ADC9 Digital Input Disable
#define DIDR2_BIT_ADC8D     0       // ADC8 Digital Input Disable

//
// NOTE there is no DIDR1
//

// DIDR0 bit definitions
#define DIDR0_BIT_ADC7D     7       // ADC7 Digital Input Disable
#define DIDR0_BIT_ADC6D     6       // ADC6 Digital Input Disable
#define DIDR0_BIT_ADC5D     5       // ADC5 Digital Input Disable
#define DIDR0_BIT_ADC4D     4       // ADC4 Digital Input Disable
#define DIDR0_BIT_ADC3D     3       // ADC3 Digital Input Disable
#define DIDR0_BIT_ADC2D     2       // ADC2 Digital Input Disable
#define DIDR0_BIT_ADC1D     1       // ADC1 Digital Input Disable
#define DIDR0_BIT_ADC0D     0       // ADC0 Digital Input Disable

#else
#error Unsupported
#endif

#endif // __ADC_H__