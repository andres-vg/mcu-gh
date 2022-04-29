#ifndef __POTLED_H__
#define __POTLED_H__

#define b2m(b) (1<<(b))

#if defined(__AVR_ATmega328P__)

#define ISR_Timer1_CompB    __vector_ ## 12
#define ISR_Twi             __vector_ ## 24

#define EXT_PIN_OC1A        1       // PORTB bit 1, board pin 9

#define TWI_LOCAL_ADDRESS   0x65    // Own slave address
#define TWI_REMOTE_ADDRESS  0x6a    // Address to whom send data

#elif defined(__AVR_ATmega2560__)

#define ISR_Timer1_CompB    __vector_ ## 18
#define ISR_Twi             __vector_ ## 39

#define EXT_PIN_OC1A        5       // PORTB bit 5, board pin 11

#define TWI_LOCAL_ADDRESS   0x6a    // Slave address
#define TWI_REMOTE_ADDRESS  0x65    // Address to whom send data

#else
#error Unsupported 
#endif

#endif // __POTLED_H__