The following directories contain the code I wrote between late 2021 and early 2022 to learn 
the programming of the __ATMega328P__ (Arduino Uno) and __ATMega2560__ (Arduino Mega 2560).

As my goal was to learn about the MCU hardware, I purposely avoided the use of the 
Arduino libraries (as much as practical).

The directories are listed here in chronological as well as complexity order.

# Hardware setup:
Devices Uno and Mega 2560 were connected to PC via USB ports (no
external power supply used).<br> 
For some experiments, the following hardware was added on a breadboard: 

<ul>
<li>Connected a linear 10 KOhm potentiometer to Vcc/Gnd, and its
center terminal to ADC0 pin (for ADC).</li>
<li> Connected external LED with 1 KOhm current limiting resistor on OC1A (to control duty cycle). 
  At 1 KHz, the perceived LED brightness is near its maximum at 20% duty cycle.</li>
<li> Interconnected both boards via their TWI (I2C) terminals, added to I2C bus one 10 KOhm 
  pull-up resistor to each line.</li>
</ul>

# [tmega-pwm](https://github.com/andres-vg/mcu-gh/tree/mcu-gh/Projects/tmega-pwm)
  Program the 3 different PWM modes into 3 different timers and observe the wave form generated.
  Timer 1 (Fast PWM mode) makes the external LED pulsate.

# [tmega-adc](https://github.com/andres-vg/mcu-gh/tree/mcu-gh/Projects/tmega-adc)
  Program timer 1 to trigger an AD conversion every 100ms. ADC programmed in auto trigger 
  (from timer 1 OC1B) and its ISR reads the AD converted value which is logged via the serial monitor.

# [tmega-potled](https://github.com/andres-vg/mcu-gh/tree/mcu-gh/Projects/tmega-potled)
  Program timer 1 in Fast PWM mode, OC1A used for the LED's duty cycle, and OC1B used to give the pace
  and generate an interruption every 1 ms, from the timer's ISR poll the ADC for data, also from the 
  same routine, kick off an AD conversion every 100ms.
  The AD value read is used to adjust the LED's duty cycle. Adjusting the pot will adjust the 
  LED brightness, perceived brightness is linear.

# [pot_led](https://github.com/andres-vg/mcu-gh/tree/mcu-gh/Projects/pot_led)
  In this code, I completely removed all Arduino dependencies except the Serial monitor and 
  the debugger settings, which are #ifdef'ed out by default.
  I created a simple TWI (I2C) API comprised of 3 functions to: init, receive and send, all but 
  init handled by interruptions.

  Program timer 1 in Fast PWM mode, use OCR1A to control external LED's duty cycle, and OCR1B 
  to give the AD pace.
  Each time a sufficiently different AD value is read, send it over I2C to the other board, 
  and whenever an I2C packet is received, update the local LED's duty cycle, so linear LED 
  brightness is controlled by the setting in the other board's pot.
