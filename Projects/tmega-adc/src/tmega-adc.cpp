//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Platform: Arduino Mega 2560 (rev 3)
// Objective: Exercise the ADC converter
//
// Program timer 1 (fast PWM Mode) to produce a compare match every 100ms, 
// which in turn will kick off an AD conversion.
// Program ADC in auto trigger mode, using the OC1B interruption flag, 
// also enable the interruption on capture complete, read that 
// value from ADC and log it via the serial monitor.
//
// Observations:
// In order to have the auto trigger mode with source the 
// Timer Compare Match B (OCR1B), the timer interruption needs to
// actually be anabled, so I had to provide an ISR for it which
// does nothing.
//--------------------------------------------------------------------

#include <Arduino.h>
#include <tmega.h>

// Other control registers
#define SREG_ADDR           0x5f    // AVR Status Register

// Memory mapped IO addresses for other control registers
volatile uint8_t * const pui8Sreg = (uint8_t *)SREG_ADDR;       // Register SREG
#define SREG                (*pui8Sreg)

// ADC
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

// Timer 1 (16 bits)
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
#define TCCRnB_BIT_WGMn3    4
#define TCCRnB_BIT_WGMn2    3
#define TIMSKn_BIT_OCIEnC   3
#define TIMSKn_BIT_OCIEnB   2
#define TIMSKn_BIT_OCIEnA   1
#define TMRDIV1             0x4 // CSn2, CSn1, CSn0 = 100b, Divide by 256

// Port B
#define PINB_ADDR       0x23
#define DDRB_ADDR       0x24
#define PORTB_ADDR      0x25
// Memory mapped IO addresses for Port B
volatile uint8_t * const pui8PinB  = (uint8_t *)PINB_ADDR;      // Register PINB
volatile uint8_t * const pui8DdrB  = (uint8_t *)DDRB_ADDR;      // Register DDRB
volatile uint8_t * const pui8PortB = (uint8_t *)PORTB_ADDR;     // Register PORTB
#define PINB (*pui8PinB)
#define DDRB (*pui8DdrB)
#define PORTB (*pui8PortB)

// gcc-avr recognizes some predefined names as ISRs, 
// that is why properly naming the ISR will make it be placed 
// in the right vector entry
#define ISR_Timer1_CompB        __vector_ ## 18
#define ISR_ADC_ConvComplete    __vector_ ## 29

extern "C" void ISR_Timer1_CompB(void)
__attribute__ ((signal,used,externally_visible));

extern "C" void ISR_ADC_ConvComplete(void)
__attribute__ ((signal,used,externally_visible));

void setup() 
{
    Serial.begin(9600);
    Serial.println("Starting...");

    // Configure timer 1 to produce a square waveform 
    // with 100ms period.
    //
    //      Waveform Generation Mode: WGMn3:0 = 1100b CTC mode, TOP defined by ICR1
    //      Compare Output Mode: COMnB1:0 = 01b
    //          Toggle OCnB on compare match
    //      Match value provided by OCRnB
    //      Set clock divisor to 256, to feed counter with 62.5KHz
    //      with a period of 16us
    //      Duty cycle = 50%
    //      Wave frequency = clock/(2*(1+TOP)) = clock/(2*(1+ICR1))
    //      Set a compare B value of 6250 
    // (i.e. 25000/250K=1/10s=100ms)
    // WGMn3:0 = 0100b for CTC mode with OCRnA as TOP
    // Set CTC mode and divider to 64
    TCCR1A = b2m(TCCRnA_BIT_COMnB0);
    TCCR1B = b2m(TCCRnB_BIT_WGMn3) | b2m(TCCRnB_BIT_WGMn2) | TMRDIV1;
    ICR1 = 6250-1; // Set TOP on ICR1
    OCR1B = 6250-1; // Set output compare value for Channel B
    TIMSK1 = b2m(TIMSKn_BIT_OCIEnB);

    // Configure the ADC as:
    // Reference=Vcc
    // Trigger source: Timer/Counter1 Compare Match B (ASTS2:0=101b)
    // Pre-scaler: divide by 128 (ADPS2:0=111b)
    //
    // At maximum resolution, SR=15K samples/sec.
    // Normally it takes 13 clock cycles for one conversion,
    // so we need a clock of no more than 195KHz (15K*13).
    // A clock divisor of 128 will give us 125KHz so we could have 
    // a maximum SR of 9.6K samples/sec. 
    // This would be a 8us clock period and pulse produced by
    // timer should be shorter than 13*8us=104us
    ADCSRA = b2m(ADCSRA_BIT_ADATE) |
             b2m(ADCSRA_BIT_ADPS2) | b2m(ADCSRA_BIT_ADPS1) | b2m(ADCSRA_BIT_ADPS0);
    ADCSRB = b2m(ADCSRB_BIT_ADTS2) | b2m(ADCSRB_BIT_ADTS0);
    ADMUX = b2m(ADMUX_BIT_REFS0); // Internal Vcc ref, Right adjust, ADC0
    DIDR0 = ~b2m(DIDR0_BIT_ADC0D); // 7:1 disabled, 0 enabled
    DIDR2 = 0xff; // 15:8 disabled
    ADCSRA |= b2m(ADCSRA_BIT_ADEN) | b2m(ADCSRA_BIT_ADIE);

    // For verification purposes, let's expose PORTB6 (OC1B)
    // All with pullup resistor except 6
    PORTB = b2m(6);
    // Only bit 6 is output
    DDRB = b2m(6);

    sei();
}

void loop() 
{
}

// Timer 1 Compare Match B
// This ISR is only required because interrupts for it MUST be enabled
// in order for the ADC auto trigger to actually trigger a conversion
// on Timer 1 Compare Match B.
void ISR_Timer1_CompB(void)
{
}

// ADC Conversion Complete
void ISR_ADC_ConvComplete(void)
{
    static uint16_t old_data = (uint16_t)-1;
    uint16_t data, diff;

    data = ADC;
    if (data >= old_data)
    {
        diff = data - old_data;
    }
    else
    {
        diff = old_data - data;
    }

    // Only log if difference greater than 3 LSB
    if (diff & ~0x7)
    {
        Serial.print('*');
        Serial.println(data);
        old_data = data;
    }
}