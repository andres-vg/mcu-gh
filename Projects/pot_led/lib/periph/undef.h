#ifndef __UNDEF_H__
#define __UNDEF_H__

// Bits manipulation
#define b2m(b) (1<<(b))

// Miscellaneous
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

// PORT B
#ifdef PINB
#undef PINB
#endif

#ifdef DDRB
#undef DDRB
#endif

#ifdef PORTB
#undef PORTB
#endif

// PORT C
#ifdef PINC
#undef PINC
#endif

#ifdef DDRC
#undef DDRC
#endif

#ifdef PORTC
#undef PORTC
#endif

// PORT D
#ifdef PIND
#undef PIND
#endif

#ifdef DDRD
#undef DDRD
#endif

#ifdef PORTD
#undef PORTD
#endif

// PORT E
#ifdef PINE
#undef PINE
#endif

#ifdef DDRE
#undef DDRE
#endif

#ifdef PORTE
#undef PORTE
#endif

// PORT H
#ifdef PINH
#undef PINH
#endif

#ifdef DDRH
#undef DDRH
#endif

#ifdef PORTH
#undef PORTH
#endif

// Timer 1
#ifdef TCCR1A // Register
#undef TCCR1A
#endif

#ifdef TCCR1B // Register
#undef TCCR1B
#endif

#ifdef OCR1A // Register
#undef OCR1A
#endif

#ifdef OCR1B // Register
#undef OCR1B
#endif

#ifdef ICR1 // Register
#undef ICR1
#endif

#ifdef TIMSK1 // Register
#undef TIMSK1
#endif

// Timer 3
#ifdef TCCR3A // Register
#undef TCCR3A
#endif

#ifdef TCCR3B // Register
#undef TCCR3B
#endif

#ifdef OCR3A // Register
#undef OCR3A
#endif

#ifdef TIMSK3 // Register
#undef TIMSK3
#endif

// Timer 4
#ifdef TCCR4A // Register
#undef TCCR4A
#endif

#ifdef TCCR4B // Register
#undef TCCR4B
#endif

#ifdef OCR4A // Register
#undef OCR4A
#endif

#ifdef TIMSK4 // Register
#undef TIMSK4
#endif

// TWI
#ifdef TWBR
#undef TWBR
#endif

#ifdef TWSR
#undef TWSR
#endif

#ifdef TWAR
#undef TWAR
#endif

#ifdef TWDR
#undef TWDR
#endif

#ifdef TWCR
#undef TWCR
#endif

#ifdef TWAMR
#undef TWAMR
#endif

// Status register
#ifdef SREG // Register
#undef SREG
#endif

// General Timer/Counter Control Register
#ifdef GTCCR // Register
#undef GTCCR
#endif

// Power Reduction Register
#ifdef PRR // Register
#undef PRR
#endif

#ifdef PRR0 // Register
#undef PRR0
#endif

// ADC
#ifdef ADC
#undef ADC
#endif

#ifdef ADCH
#undef ADCH
#endif

#ifdef ADCL
#undef ADCL
#endif

#ifdef ADCSRA
#undef ADCSRA
#endif

#ifdef ADCSRB
#undef ADCSRB
#endif

#ifdef ADMUX
#undef ADMUX
#endif

#ifdef DIDR2
#undef DIDR2
#endif

#ifdef DIDR0
#undef DIDR0
#endif

#endif // __UNDEF_H__