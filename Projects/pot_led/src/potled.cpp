// Arduino Mega 2560 + Elego Uno R3 (Arduino Uno R3 compatible)
// 
// The goal is to have both boards, using about the same code, 
// reading the voltage from a potentiometer, and sending it 
// via TWM (I2C) to the other board to control its external
// LED's brightness
//
// NOTES:
// In this experiment, I have separated each "peripheral"'s definitions
// into their own file (e.g. timer.*, adc.*, twi*.*). Only TWI has actual
// code and an API defined in separate files.
//
// I also have added macros to use the serial debugger with 2 levels
// of verbosity (see dbg.h)
#include <stdint.h>

#include <dbg.h>
#include <undef.h>
#include <gpio.h>
#include <timer.h>
#include <adc.h>
#include <twiapi.h>
#include <potled.h>

#if __USE_AVR8_STUB__
#include <avr_debugger.h>
#endif


// gcc-avr recognizes some predefined names as ISRs, 
// that is why properly naming the ISR will make it being placed 
// in the right vector entry
#ifdef __cplusplus
extern "C" {
#endif
    void ISR_Timer1_CompB(void)
    __attribute__ ((signal,used,externally_visible));

     void setup(void)
    __attribute__ ((used,externally_visible));

    void loop(void)
    __attribute__ ((used,externally_visible));
#ifdef __cplusplus
}
#endif

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
    twiRxBuf_t recvBuf;
    twiTxBuf_t sendBuf;
    

    // First read AD data if available
    if (ADCSRA & b2m(ADCSRA_BIT_ADIF))
    {
        // We have an AD sample

        // We will use 6 MSB, so values go between 0 and 63
        data = (uint8_t)((ADC >> 4) & 0x3f);

        if (data != old_data)
        {
            sendBuf.toAddr = TWI_REMOTE_ADDRESS;
            sendBuf.buffer[0] = 'I';
            sendBuf.buffer[1] = '2';
            sendBuf.buffer[2] = 'C';
            sendBuf.buffer[3] = data;
            sendBuf.len = 4;
            if (twiSend(&sendBuf))
            {
                // If (starting a) send succeeded, update old_data, 
                // otherwise don't so we try again on next 
                // ADC conversion
                old_data = data;
                SerialPr(("Starting packet send "));
                SerialPrLn((data));
            }
            else
            {
                SerialPrLn(("Send packet discarded"));
            }
        }

        // Manually reset interrupt flag as we don't have 
        // ADC interrupts enabled nor corresponding ISR
        ADCSRA |= b2m(ADCSRA_BIT_ADIF);
    }

    // Find out if we have received any data
    if (twiRecv(&recvBuf) && recvBuf.size == 4)
    {
        // ... and we have.
        // Update duty cycle
        OCR1A = step[recvBuf.buffer[3] & 0x3f];
        SerialPrLn(("Received packet"));
    }

    count++;
    if (count >= 100)
    {
        count = 0;
        ADCSRA |= b2m(ADCSRA_BIT_ADSC); // Start a new conversion
    }
}

void setup(void)
{
    dbg_init();
    SerialBegin((9600));
    SerialPr(("Starting: "));
    SerialPrLn((BOARD_NAME));

    // Configure timer 1 (16 bits) - Fast PWM mode
    //
    //      Waveform Generation Mode: WGMn3:0 = 1110b Fast PWM mode, 
    //      TOP defined by ICR1
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
    OCR1A = (2000*1/100)-1; // Initial duty cycle 1%
    OCR1B = 2000/2; // Set (mid way) where we will get OC1B interrupt
    TIMSK1 = b2m(TIMSKn_BIT_OCIEnB);

    // All with pullup resistor except the pin where the
    // external LED is connected
    PORTB = b2m(EXT_PIN_OC1A);
    // Only bit with external LED is output
    DDRB = b2m(EXT_PIN_OC1A);

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
#if defined(__AVR_ATmega2560__)
    DIDR2 = 0xff;                  // 15:8 disabled
#endif
    ADCSRA |= b2m(ADCSRA_BIT_ADEN) | // Enable AD converter
              b2m(ADCSRA_BIT_ADSC);  // Start an AD conversion

    dbg_breakpoint();
    twiInit(TWI_LOCAL_ADDRESS);

    asm volatile("sei" ::);
}

void loop(void)
{
}