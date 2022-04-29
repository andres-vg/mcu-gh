#ifndef __DBG_H__
#define __DBG_H__

// Control the avr-stub debugger
#define __USE_AVR8_STUB__       0

// Control the serial spew
#define __USE_DEBUG_SPEW__      0

#if defined(__AVR_ATmega328P__)
#define BOARD_NAME              "Arduino Uno"
#elif defined(__AVR_ATmega2560__)
#define BOARD_NAME              "Mega 2560"
#else
#define BOARD_NAME              "Unsupported"
#endif

#if __USE_DEBUG_SPEW__
//#include <avr/pgmspace.h>
#include <HardwareSerial.h>
#define SerialBegin(params) Serial.begin params
#define SerialPr(params) Serial.print  params
#define SerialPrLn(params) Serial.println  params 
#if __USE_DEBUG_SPEW__ > 1
#define SerialPr2(params) Serial.print  params
#define SerialPrLn2(params) Serial.println  params 
#else
#define SerialPr2(params)
#define SerialPrLn2(params)
#endif
#else
#define SerialBegin(params)
#define SerialPr(params)
#define SerialPrLn(params)
#define SerialPr2(params)
#define SerialPrLn2(params)
#endif

#if !__USE_AVR8_STUB__
#define dbg_init(x)
#define dbg_breakpoint(x)
#endif

#endif // __DBG_H__