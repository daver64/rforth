\ Software PWM Fade Example
\ Hardware: LED and 220 ohm resistor between GPIO18 and GND
\ Note: This is software PWM, not hardware PWM

\ Initialize GPIO
GPIO-INIT

\ Configure GPIO18 as output
18 GPIO-OUTPUT

\ Define PWM period in microseconds
1000 CONSTANT PWM-PERIOD

\ Define software PWM word
: SOFT-PWM ( duty-us -- )
  PWM-PERIOD OVER - SWAP    \ Calculate off-time and on-time
  18 GPIO-SET               \ Turn LED on
  DELAY-US                  \ Wait duty time
  18 GPIO-CLR               \ Turn LED off
  DELAY-US ;                \ Wait off time

\ Fade up
: FADE-UP
  PWM-PERIOD 0 DO
    I SOFT-PWM
  10 +LOOP ;

\ Fade down
: FADE-DOWN
  PWM-PERIOD 0 DO
    PWM-PERIOD I - SOFT-PWM
  10 +LOOP ;

\ Fade demo
: FADE-DEMO
  ." Fading LED up and down 3 times..." CR
  3 0 DO
    FADE-UP
    FADE-DOWN
  LOOP
  18 GPIO-CLR ;

\ Run the demo
FADE-DEMO

\ Cleanup
GPIO-CLOSE

CR ." Fade demo complete!" CR
