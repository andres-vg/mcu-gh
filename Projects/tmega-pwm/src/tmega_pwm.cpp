#include <Arduino.h>
#include <tmega.h>
#include <avr_debugger.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Platform: Arduino Mega 2560 (rev 3)
// Objective: Exercise the timers PWM modes
//
// Program timer 1 (fast PWM Mode)
//      Make an external LED pulsate, observe duty cycle changes.
// Program timer 3 (Phase Correct PWM Mode) 
//      Alternate duty cycle and observe in oscilloscope.
// Program timer 4 (Phase and Frequency Correct PWM Mode)
//      Alternate duty cycle and observe in oscilloscope.
//
// Timer 1 output compare A pin is in PORTB bit 5 exposed on Board pin 11
// Timer 3 output compare A pin is in PORTE bit 3 exposed on Board pin 5
// Timer 4 output compare A pin is in PORTH bit 3 exposed on Board pin 6
//
// Observations:
//--------------------------------------------------------------------

// PORTB
#define PINB_ADDR       0x23
#define DDRB_ADDR       0x24
#define PORTB_ADDR      0x25
#define PORTB_BIT_OC1A  5       // PORTB bit 5, timer 1 output compare A pin (Board pin 11)

// PORTE
#define PINE_ADDR       0x2c
#define DDRE_ADDR       0x2d
#define PORTE_ADDR      0x2e
#define PORTE_BIT_OC3A  3       // PORTE bit 3, timer 3 output compare A pin (Board pin 5)

// PORTH
#define PINH_ADDR       0x100
#define DDRH_ADDR       0x101
#define PORTH_ADDR      0x102
#define PORTH_BIT_OC4A  3       // PORTH bit 3, timer 4 output compare A pin (Board pin 6)

// NOTE Timers 1, 3, 4, and 5 are identical, so bits in registers are the same position

// Timer 1 (16 bits) will be used
#define TCCR1A_ADDR     0x80    // Timer/Counter1 Control Register A
#define TCCR1B_ADDR     0x81    // Timer/Counter1 Control Register B
#define OCR1A_ADDR      0x88    // Output Compare Register 1 A (16 bits register)
#define TIMSK1_ADDR     0x6f    // Timer/Counter1 Interrupt Mask Register
#define ICR1_ADDR       0x86    // Input Capture Register 1

// Timer 3 (16 bits) will be used
#define TCCR3A_ADDR     0x90    // Timer/Counter1 Control Register A
#define TCCR3B_ADDR     0x91    // Timer/Counter1 Control Register B
#define OCR3A_ADDR      0x98    // Output Compare Register 1 A (16 bits register)
#define TIMSK3_ADDR     0x71    // Timer/Counter1 Interrupt Mask Register

// Timer 4 (16 bits) will be used
#define TCCR4A_ADDR     0xa0    // Timer/Counter1 Control Register A
#define TCCR4B_ADDR     0xa1    // Timer/Counter1 Control Register B
#define OCR4A_ADDR      0xa8    // Output Compare Register 1 A (16 bits register)
#define TIMSK4_ADDR     0x72    // Timer/Counter1 Interrupt Mask Register


#define SREG_ADDR       0x5f    // AVR Status Register
#define GTCCR_ADDR      0x43    // General Timer/Counter Control Register

// Memory mapped IO addresses for IO port B
volatile uint8_t * const pui8PinB  = (uint8_t *)PINB_ADDR;      // Register PINB
volatile uint8_t * const pui8DdrB  = (uint8_t *)DDRB_ADDR;      // Register DDRB
volatile uint8_t * const pui8PortB = (uint8_t *)PORTB_ADDR;     // Register PORTB
#define PINB (*pui8PinB)
#define DDRB (*pui8DdrB)
#define PORTB (*pui8PortB)

// Memory mapped IO addresses for IO port E
volatile uint8_t * const pui8PinE  = (uint8_t *)PINE_ADDR;      // Register PINE
volatile uint8_t * const pui8DdrE  = (uint8_t *)DDRE_ADDR;      // Register DDRE
volatile uint8_t * const pui8PortE = (uint8_t *)PORTE_ADDR;     // Register PORTE
#define PINE (*pui8PinE)
#define DDRE (*pui8DdrE)
#define PORTE (*pui8PortE)

// Memory mapped IO addresses for IO port H
volatile uint8_t * const pui8PinH  = (uint8_t *)PINH_ADDR;      // Register PINH
volatile uint8_t * const pui8DdrH  = (uint8_t *)DDRH_ADDR;      // Register DDRH
volatile uint8_t * const pui8PortH = (uint8_t *)PORTH_ADDR;     // Register PORTH
#define PINH (*pui8PinH)
#define DDRH (*pui8DdrH)
#define PORTH (*pui8PortH)

// Memory mapped IO addresses for timer 1
volatile uint8_t * const pui8Tccr1A = (uint8_t *)TCCR1A_ADDR;   // Register TCCR1A
volatile uint8_t * const pui8Tccr1B = (uint8_t *)TCCR1B_ADDR;   // Register TCCR1B
volatile uint16_t * const pui16Ocr1A = (uint16_t *)OCR1A_ADDR;  // Register OCR1A
volatile uint8_t * const pui8Timsk1 = (uint8_t *)TIMSK1_ADDR;   // Register TIMSK1
volatile uint16_t * const pui16Icr1 = (uint16_t *)ICR1_ADDR;    // Register OCR1A
#define TCCR1A (*pui8Tccr1A)
#define TCCR1B (*pui8Tccr1B)
#define OCR1A (*pui16Ocr1A)
#define TIMSK1 (*pui8Timsk1)
#define ICR1 (*pui16Icr1)

// Memory mapped IO addresses for timer 3
volatile uint8_t * const pui8Tccr3A = (uint8_t *)TCCR3A_ADDR;   // Register TCCR3A
volatile uint8_t * const pui8Tccr3B = (uint8_t *)TCCR3B_ADDR;   // Register TCCR3B
volatile uint16_t * const pui16Ocr3A = (uint16_t *)OCR3A_ADDR;  // Register OCR3A
volatile uint8_t * const pui8Timsk3 = (uint8_t *)TIMSK3_ADDR;   // Register TIMSK3
#define TCCR3A (*pui8Tccr3A)
#define TCCR3B (*pui8Tccr3B)
#define OCR3A (*pui16Ocr3A)
#define TIMSK3 (*pui8Timsk3)

// Memory mapped IO addresses for timer 4
volatile uint8_t * const pui8Tccr4A = (uint8_t *)TCCR4A_ADDR;   // Register TCCR4A
volatile uint8_t * const pui8Tccr4B = (uint8_t *)TCCR4B_ADDR;   // Register TCCR4B
volatile uint16_t * const pui16Ocr4A = (uint16_t *)OCR4A_ADDR;  // Register OCR4A
volatile uint8_t * const pui8Timsk4 = (uint8_t *)TIMSK4_ADDR;   // Register TIMSK4
#define TCCR4A (*pui8Tccr4A)
#define TCCR4B (*pui8Tccr4B)
#define OCR4A (*pui16Ocr4A)
#define TIMSK4 (*pui8Timsk4)

// Memory mapped IO addresses for other control registers
volatile uint8_t * const pui8Sreg = (uint8_t *)SREG_ADDR;       // Register SREG
volatile uint8_t * const pui8Gtccr = (uint8_t *)GTCCR_ADDR;     // Register GTCCR
#define SREG (*pui8Sreg)
#define GTCCR (*pui8Gtccr)

// Timer n registers, bit definitions (timers 1, 3, 4 and 5 are identical)
#define TCCRnA_BIT_COMnA1   7
#define TCCRnA_BIT_COMnA0   6
#define TCCRnA_BIT_WGMn1    1
#define TCCRnA_BIT_WGMn0    0
#define TCCRnB_BIT_WGMn3    4
#define TCCRnB_BIT_WGMn2    3
#define TIMSKn_BIT_TOIEn    0   // Timer/Counter n, Overflow Interrupt Enable
#define TIMSKn_BIT_OCIEnA   1   // Timer/Counter n, Output Compare A Match Interrupt Enable
#define TMRDIV1             0x3 // CSn2, CSn1, CSn0 = 011b, Divide by 64
#define TMRDIV3             0x2 // CSn2, CSn1, CSn0 = 010b, Divide by 8
#define TMRDIV4             0x2 // CSn2, CSn1, CSn0 = 010b, Divide by 8

// gcc-avr recognizes some predefined names as ISRs, 
// that is why properly naming the ISR will make it be placed 
// in the right vector entry
#define ISR_Timer1_CompA    __vector_ ## 17
#define ISR_Timer3_Tovf     __vector_ ## 35
#define ISR_Timer4_Tovf     __vector_ ## 45

extern "C" void ISR_Timer1_CompA(void)
__attribute__ ((signal,used,externally_visible));

extern "C" void ISR_Timer3_Tovf(void)
__attribute__ ((signal,used,externally_visible));

extern "C" void ISR_Timer4_Tovf(void)
__attribute__ ((signal,used,externally_visible));


// GTCCR General Timer/Counter Control Register, bit definitions
#define GTCCR_BIT_TSM       7   // TSM: Timer/Counter Synchronization Mode
#define GTCCR_BIT_PSRASY    1   // PSRASY: Prescaler Reset Timer/Counter2
#define GTCCR_BIT_PSRSYNC   0   // PSRSYNC: Prescaler Reset for Synchronous Timer/Counters

void setup() 
{
    debug_init();

    cli();
    // Prevent timers from running
    GTCCR = b2m(GTCCR_BIT_TSM)|b2m(GTCCR_BIT_PSRASY)|b2m(GTCCR_BIT_PSRSYNC);

    // Configure IO ports, all pins with pull-up resistor,
    // except the following bits:
    // B 5 for the comparator A output pin of timer 1 (Board pin 11)
    // E 3 for the comparator A output pin of timer 3 (Board pin 5)
    // H 3 for the comparator A output pin of timer 4 (Board pin 6)
    PORTB = b2m(PORTB_BIT_OC1A);
    PORTE = b2m(PORTE_BIT_OC3A);
    PORTH = b2m(PORTH_BIT_OC4A);

    // All IO port pins set as inputs, except the bits on which we want to 
    // see the comparator A output of each timer
    DDRB = b2m(PORTB_BIT_OC1A);
    DDRE = b2m(PORTE_BIT_OC3A);
    DDRH = b2m(PORTH_BIT_OC4A);

    // Configure timer 1 (16 bits) - Fast PWM mode
    //      Waveform Generation Mode: WGMn3:0 = 1110b Fast PWM mode, TOP defined by ICR1
    //      Compare Output Mode: COMnA1:0 = 10b
    //          Clear OCnA on match
    //          Set OCnA at BOTTOM
    //      (non-inverting)
    //      Match value provided by OCRnA
    //      Interrupt compare will happen with match, and 
    //          right there I will update the compare value (when needed)
    //      Set clock devisor to 64, to feed counter with 250KHz
    //      Duty cycle = MatchA/(1+TOP) = OCR1A/(1+ICR1)
    //      PWM frequency = clock/(1+TOP) = clock /(1+ICR1)
    TCCR1A = b2m(TCCRnA_BIT_COMnA1) | b2m(TCCRnA_BIT_WGMn1);
    TCCR1B = b2m(TCCRnB_BIT_WGMn3) | b2m(TCCRnB_BIT_WGMn2) | TMRDIV1;
    ICR1 = 249; // Set TOP, other modes can be used that predefine TOP to 0xff, 0x1ff or 0x3ff
    OCR1A = 25; // Duty cycle 10%, freq ~= 1000 Hz
    TIMSK1 = b2m(TIMSKn_BIT_OCIEnA);

    // Configure timer 3 (16 bits) - Phase Correct PWM Mode
    //      Waveform Generation Mode: WGMn3:0 = 0011b Phase Correct PWM mode, TOP predefined 0x03ff
    //      Compare Output Mode: COMnA1:0 = 10b
    //          Clear OCnA on match when up-counting
    //          Set OCnA on match when down-counting
    //      Match value provided by OCRnA
    //      Interrupt on overflow will happen at bottom, and
    //          right there I will update the compare value (when needed)
    //      Set clock devisor to 8, to feed counter with 2MHz
    //      Duty cycle = MatchA/TOP = OCR3A/0x03ff
    //      PWM frequency = clock/(2*TOP) = clock /(2*0x03ff)
    TCCR3A = b2m(TCCRnA_BIT_COMnA1) | b2m(TCCRnA_BIT_WGMn1) | b2m(TCCRnA_BIT_WGMn0);
    TCCR3B = TMRDIV3;
    OCR3A = 0x100; // Duty cycle 25% , freq ~= 977 Hz
    TIMSK3 = b2m(TIMSKn_BIT_TOIEn);

    // Configure timer 4 (16 bits) - Phase and Frequency Correct PWM Mode
    //      Waveform Generation Mode: WGMn3:0 = 1000b Phase and Frequency Correct PWM mode,
    //          TOP defined by ICR4
    //      Compare Output Mode: COMnA1:0 = 10b
    //          Clear OCnA on match when up-counting
    //          Set OCnA on match when down-counting
    //      Match value provided by OCRnA
    //      Interrupt on overflow will happen at bottom, and
    //          right there I will update the compare value (when needed)
    //      Set clock devisor to 8, to feed counter with 2MHz
    //      Duty cycle = MatchA/TOP = OCR4A/ICR4
    //      PWM frequency = clock/(2*TOP) = clock /(2*ICR4)
    TCCR4A = b2m(TCCRnA_BIT_COMnA1);
    TCCR4B = b2m(TCCRnB_BIT_WGMn3) | TMRDIV4;
    ICR4 = 1000;
    OCR4A = 250; // Duty cycle 25% , freq ~= 1000 Hz
    TIMSK4 = b2m(TIMSKn_BIT_TOIEn);

    sei();
    GTCCR = 0;
}

void loop() 
{
}

// Timer/Counter 1, Compare Match A
void ISR_Timer1_CompA(void)
{
    static volatile bool goingUp = true;
    static volatile unsigned int count = 0;
    // These are the values to apply OCR1A to set the duty cycle to:
    // 1%, 2%, .., 20%
    // Every count move to the next step then at the end, move backwards
    static const uint16_t tmr1Steps[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 12, 13, 15, 18, 20, 23, 27, 32, 38, 50
    };

    static volatile int step = 1;

    count++;
    if (count >= 20)
    {
        OCR1A = tmr1Steps[step];
        count = 0;

        if (goingUp)
        {
            step++;
            if (step >= (int)ARRAY_SIZE(tmr1Steps))
            {
                step = ARRAY_SIZE(tmr1Steps) - 2;
                goingUp = false;
            }
        }
        else
        {
            step--;
            if (step < 0)
            {
                step = 1;
                goingUp = true;
            }
        }
    }
}

// Timer/Counter 3, overflow
void ISR_Timer3_Tovf(void)
{
    static volatile unsigned int count = 0;
    static volatile bool dc25 = true;

    count++;
    if (count >= 1000)
    {
        count = 0;
        if (dc25)
        {
            // Change to duty cycle 75
            OCR3A = 0x300;
            dc25 = false;
        }
        else
        {
            // Change to duty cycle 25
            OCR3A = 0x100;
            dc25 = true;
        }
    }
}

// Timer/Counter 4, overflow
void ISR_Timer4_Tovf(void)
{
    static volatile unsigned int count = 0;
    static volatile bool dc25 = true;

    count++;
    if (count >= 1000)
    {
        count = 0;
        if (dc25)
        {
            // Change to duty cycle 75
            OCR4A = 750;
            dc25 = false;
        }
        else
        {
            // Change to duty cycle 25
            OCR4A = 250;
            dc25 = true;
        }
    }
}