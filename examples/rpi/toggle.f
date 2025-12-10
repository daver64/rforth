\ Toggle LED Example
\ Hardware: LED and 220 ohm resistor between GPIO17 and GND

\ Initialize GPIO
GPIO-INIT

\ Configure GPIO17 as output
17 GPIO-OUTPUT

\ Define toggle-demo word
: TOGGLE-DEMO
  ." Toggling LED 20 times..." CR
  20 0 DO
    17 GPIO-TOGGLE    \ Toggle LED state
    250 DELAY-MS      \ Wait 250ms
  LOOP ;

\ Run the demo
TOGGLE-DEMO

\ Ensure LED is off at end
17 GPIO-CLR

\ Cleanup
GPIO-CLOSE

CR ." Toggle demo complete!" CR
