\ Traffic Light Simulator
\ Hardware:
\   - Red LED on GPIO17
\   - Yellow LED on GPIO22
\   - Green LED on GPIO27
\   Each with 220 ohm resistor to GND

\ Initialize GPIO
GPIO-INIT

\ Configure pins as outputs
17 GPIO-OUTPUT    \ Red
22 GPIO-OUTPUT    \ Yellow
27 GPIO-OUTPUT    \ Green

\ Define color constants
17 CONSTANT RED
22 CONSTANT YELLOW
27 CONSTANT GREEN

\ Define light control words
: ALL-OFF
  RED GPIO-CLR
  YELLOW GPIO-CLR
  GREEN GPIO-CLR ;

: RED-LIGHT
  ALL-OFF
  RED GPIO-SET
  3000 DELAY-MS ;

: YELLOW-LIGHT
  ALL-OFF
  YELLOW GPIO-SET
  1000 DELAY-MS ;

: GREEN-LIGHT
  ALL-OFF
  GREEN GPIO-SET
  3000 DELAY-MS ;

\ Traffic light sequence
: TRAFFIC-SEQUENCE
  RED-LIGHT
  RED GPIO-SET      \ Keep red on
  YELLOW GPIO-SET   \ Add yellow
  1000 DELAY-MS
  GREEN-LIGHT
  YELLOW-LIGHT ;

\ Run traffic light cycles
: TRAFFIC-DEMO
  ." Running traffic light (5 cycles)..." CR
  5 0 DO
    TRAFFIC-SEQUENCE
  LOOP
  ALL-OFF ;

\ Run the demo
TRAFFIC-DEMO

\ Cleanup
GPIO-CLOSE

CR ." Traffic light demo complete!" CR
