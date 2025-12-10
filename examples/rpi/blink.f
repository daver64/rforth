\ Simple LED Blink Example
\ Hardware: Connect LED and 220 ohm resistor between GPIO17 and GND

\ Initialize GPIO subsystem
GPIO-INIT

\ Configure GPIO17 as output
17 GPIO-OUTPUT

\ Define blink word
: BLINK-LED
  10 0 DO
    17 GPIO-SET        \ Turn LED on
    500 DELAY-MS       \ Wait 500ms
    17 GPIO-CLR        \ Turn LED off
    500 DELAY-MS       \ Wait 500ms
  LOOP ;

\ Run the blink
BLINK-LED

\ Cleanup
GPIO-CLOSE

\ Exit message
CR ." Blink complete!" CR
