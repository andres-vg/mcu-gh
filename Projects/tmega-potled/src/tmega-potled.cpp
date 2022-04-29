//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Platform: Arduino Mega 2560 (rev 3)
// Objective: Control an external LED brightness with a potentiometer.
// The pot will be connected to Vcc/GND and the center to an ADC input.
// A timer will be used to light the LED with a duty cycle controlled
// by the voltage read in the pot center lead.
//
// I will use Timer 1 in Fast PWM mode producing in OC1A a 1KHz signal.
//
// OC1A uses PORT B, bit 5, on board pin 11; this will drive the LED.
//
// OC1B interrupt will be used to read ADC and start a new conversion.
// I will be reading voltage at 10 times/second
//
// The duty cycle in OC1A will control LED brightness.
// In turn duty cycle will be controlled by the voltage read from 
// the pot through the ADC in ADC0.
//
// NOTE I noticed that a duty cycle of 20% lightens LED at pretty much
// full brightness, so the control will be 0.1-20% duty cycle.
// As brightness is not lineal, I will use a table to convert from the 
// pot step to the actual duty cycle following an exponential function.
#include <Arduino.h>
#include <tmega.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Other control registers
//------------------------------------------------------------------------
#define SREG_ADDR           0x5f    // AVR Status Register
// Memory mapped IO addresses for other control registers
volatile uint8_t * const pui8Sreg = (uint8_t *)SREG_ADDR;       // Register SREG
#define SREG                (*pui8Sreg)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Port B
//------------------------------------------------------------------------
#define PINB_ADDR           0x23
#define DDRB_ADDR           0x24
#define PORTB_ADDR          0x25
// Memory mapped IO addresses for Port B
volatile uint8_t * const pui8PinB  = (uint8_t *)PINB_ADDR;      // Register PINB
volatile uint8_t * const pui8DdrB  = (uint8_t *)DDRB_ADDR;      // Register DDRB
volatile uint8_t * const pui8PortB = (uint8_t *)PORTB_ADDR;     // Register PORTB
#define PINB (*pui8PinB)
#define DDRB (*pui8DdrB)
#define PORTB (*pui8PortB)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Timer 1 (16 bits)
//------------------------------------------------------------------------
#define TCCR1A_ADDR         0x80    // Timer/Counter1 Control Register A
#define TCCR1B_ADDR         0x81    // Timer/Counter1 Control Register B
#define OCR1A_ADDR          0x88    // Output Compare Register 1 A
#define OCR1B_ADDR          0x8a    // Output Compare Register 1 B
#define ICR1_ADDR           0x86    // Input Capture Register 1
#define TIMSK1_ADDR         0x6f    // Timer/Counter 1 Interrupt Mask Register
// Memory mapped IO addresses for Timer 1
volatile uint8_t * const pui8Tccr1A = (uint8_t *)TCCR1A_ADDR;   // Register TCCR1A
volatile uint8_t * const pui8Tccr1B = (uint8_t *)TCCR1B_ADDR;   // Register TCCR1B
volatile uint16_t * const pui16Ocr1A = (uint16_t *)OCR1A_ADDR;  // Register OCR1B
volatile uint16_t * const pui16Ocr1B = (uint16_t *)OCR1B_ADDR;  // Register OCR1B
volatile uint16_t * const pui16Icr1 = (uint16_t *)ICR1_ADDR;    // Register ICR1
volatile uint8_t * const pui8Timsk1 = (uint8_t *)TIMSK1_ADDR;   // Register TIMSK1
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
//
#define TCCRnB_BIT_WGMn3    4
#define TCCRnB_BIT_WGMn2    3
#define TCCRnB_BIT_WGMn2    3
#define TCCRnB_DIV1         0x1
#define TCCRnB_DIV8         0x2
#define TCCRnB_DIV64        0x3
#define TCCRnB_DIV256       0x4
#define TCCRnB_DIV1024      0x5
//
#define TIMSKn_BIT_OCIEnC   3
#define TIMSKn_BIT_OCIEnB   2
#define TIMSKn_BIT_OCIEnA   1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADC
//------------------------------------------------------------------------
#define ADC_ADDR            0x78    // The ADC Data Register
#define ADCH_ADDR           0x79    // The ADC Data Register
#define ADCL_ADDR           0x78    // The ADC Data Register
#define ADCSRA_ADDR         0x7a    // ADC Control and Status Register A
#define ADCSRB_ADDR         0x7b    // ADC Control and Status Register B
#define ADMUX_ADDR          0x7c    // ADC Multiplexer Selection Register
#define DIDR2_ADDR          0x7d    // Digital Input Disable Register 2, NOTE There is no DIDR1
#define DIDR0_ADDR          0x7e    // Digital Input Disable Register 0, NOTE There is no DIDR1
// Memory mapped IO addresses for ADC
volatile uint16_t * const pui16Adc  = (uint16_t *)ADC_ADDR;
volatile uint8_t * const pui8AdcH  = (uint8_t *)ADCH_ADDR;
volatile uint8_t * const pui8AdcL  = (uint8_t *)ADCL_ADDR;
volatile uint8_t * const pui8Adcsra  = (uint8_t *)ADCSRA_ADDR;
volatile uint8_t * const pui8Adcsrb  = (uint8_t *)ADCSRB_ADDR;
volatile uint8_t * const pui8Admux  = (uint8_t *)ADMUX_ADDR;
volatile uint8_t * const pui8Didr2  = (uint8_t *)DIDR2_ADDR;
volatile uint8_t * const pui8Didr0  = (uint8_t *)DIDR0_ADDR;
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

// gcc-avr recognizes some predefined names as ISRs, 
// that is why properly naming the ISR will make it be placed 
// in the right vector entry
#define ISR_Timer1_CompB        __vector_ ## 18

extern "C" void ISR_Timer1_CompB(void)
__attribute__ ((signal,used,externally_visible));


void setup() 
{
    // Configure timer 1 (16 bits) - Fast PWM mode
    //      Waveform Generation Mode: WGMn3:0 = 1110b Fast PWM mode, TOP defined by ICR1
    //      Compare Output Mode: COMnA1:0 = 10b
    //          Clear OCnA on match
    //          Set OCnA at BOTTOM
    //      Compare Output Mode: COMnB1:0 = 00b
    //          Normal port operation, OCnB disconnected
    //      Match value provided by OCRnA controls duty cycle
    //      Match value provided by OCRnB determins when to interrupt.
    //
    //      Set clock devisor to 8, to feed counter with 2MHz
    //      Duty cycle = (MatchA+1)/(1+TOP) = (OCR1A+1)/(1+ICR1)
    //      PWM frequency = clock/(1+TOP) = clock/(1+ICR1)
    TCCR1A = b2m(TCCRnA_BIT_COMnA1) | b2m(TCCRnA_BIT_WGMn1);
    TCCR1B = b2m(TCCRnB_BIT_WGMn3) | b2m(TCCRnB_BIT_WGMn2) | TCCRnB_DIV8;
    ICR1 = 2000 - 1; // Set TOP; TOP+1 clock cycles will be our OC1A period (1 ms)
    OCR1A = (2000*10/100)-1; // Initial duty cycle 10%
    OCR1B = 2000/2; // Set where we will get OC1B interrupt
    TIMSK1 = b2m(TIMSKn_BIT_OCIEnB);

    // All with pullup resistor except 5
    PORTB = b2m(5);
    // Only bit 5 is output
    DDRB = b2m(5);

    // Configure the ADC as:
    // 
    // ADCSRA   ADPS2:0 = 111b Pre-scaler divide by 128 
    // ADCSRB   ADTS2:0 = 000b Free running mode
    // ADMUX    REFS1:0 = 01b Internal Vcc reference
    // ADMUX    ADLAR = 0b Right adjust
    // ADMUX    MUX4:0 = 00000b Select analog input ADC0
    //
    // At maximum resolution, SR=15K samples/sec.
    // Normally it takes 13 clock cycles for one conversion,
    // so we need a clock of no more than 195KHz (15K*13).
    // A clock divisor of 128 will give us 125KHz so we could have 
    // a maximum SR of 9.6K samples/sec. 
    // I intend to have 10 conversion/sec, enough to be responsive
    // while moving the pot
    ADCSRA = ADCSRA_DIV128;
    ADCSRB = 0;
    ADMUX = b2m(ADMUX_BIT_REFS0);
    DIDR0 = ~b2m(DIDR0_BIT_ADC0D); // 7:1 disabled, 0 enabled
    DIDR2 = 0xff;                  // 15:8 disabled
    ADCSRA |= b2m(ADCSRA_BIT_ADEN) | // Enable AD converter
              b2m(ADCSRA_BIT_ADSC);  // Start an AD conversion

    sei();
}

void loop() 
{
}

// Timer 1 Compare Match B
void ISR_Timer1_CompB(void)
{
    uint8_t data;
    static uint8_t count = 0;
    static uint8_t old_data = (uint8_t)-1;
    // Values to apply to generate a duty cycle between 0.1 and 20%
    // It follows an exponential (see led-log.xlsx)
    static const uint8_t step[64] = {
        1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 8, 8, 
        9, 10, 11, 11, 12, 14, 15, 16, 17, 19, 21, 22, 24, 27, 29, 31, 34, 37, 40, 44, 
        48, 52, 57, 62, 67, 73, 79, 86, 94, 102, 111, 121, 131, 143, 155, 169, 184, 200
    };
    
    // First read data if available
    if (ADCSRA & b2m(ADCSRA_BIT_ADIF))
    {
        // We have a sample

        // We will use 6 MSB, so values go between 0 and 63
        data = (uint8_t)((ADC >> 4) & 0x3f); 
        if (data != old_data)
        {
            old_data = data;
            // Update duty cycle
            OCR1A = step[data];
        }

        ADCSRA |= b2m(ADCSRA_BIT_ADIF); // Reset flag by writting '1'
    }

    count++;
    if (count >= 100)
    {
        // Start a conversion every 100ms
        count = 0;
        ADCSRA |= b2m(ADCSRA_BIT_ADSC); // Start a new conversion
    }
}
