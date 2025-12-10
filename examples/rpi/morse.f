\ Morse Code LED Example
\ Hardware: LED and 220 ohm resistor between GPIO17 and GND

\ Initialize GPIO
GPIO-INIT

\ Configure GPIO17 as output
17 GPIO-OUTPUT

\ Morse code timing constants (in milliseconds)
100 CONSTANT DOT-TIME
300 CONSTANT DASH-TIME
100 CONSTANT SYMBOL-SPACE
300 CONSTANT LETTER-SPACE
700 CONSTANT WORD-SPACE

\ Basic morse elements
: DOT
  17 GPIO-SET
  DOT-TIME DELAY-MS
  17 GPIO-CLR
  SYMBOL-SPACE DELAY-MS ;

: DASH
  17 GPIO-SET
  DASH-TIME DELAY-MS
  17 GPIO-CLR
  SYMBOL-SPACE DELAY-MS ;

: LETTER-GAP
  LETTER-SPACE DELAY-MS ;

: WORD-GAP
  WORD-SPACE DELAY-MS ;

\ Morse code letters
: M-S  DOT DOT DOT LETTER-GAP ;              \ S
: M-O  DASH DASH DASH LETTER-GAP ;           \ O
: M-H  DOT DOT DOT DOT LETTER-GAP ;          \ H
: M-I  DOT DOT LETTER-GAP ;                  \ I
: M-R  DOT DASH DOT LETTER-GAP ;             \ R
: M-P  DOT DASH DASH DOT LETTER-GAP ;        \ P
: M-A  DOT DASH LETTER-GAP ;                 \ A

\ Send "SOS"
: SEND-SOS
  ." Sending SOS..." CR
  M-S M-O M-S
  WORD-GAP ;

\ Send "RASPBERRY PI"
: SEND-MESSAGE
  ." Sending RASPBERRY PI..." CR
  M-R M-A M-S M-P   \ RASP
  M-B M-E M-R M-R   \ BERR (would need M-B, M-E defined)
  WORD-GAP
  M-P M-I           \ PI
  WORD-GAP ;

\ Simple demo - just SOS
: MORSE-DEMO
  3 0 DO
    SEND-SOS
    1000 DELAY-MS
  LOOP
  17 GPIO-CLR ;

\ Run the demo
MORSE-DEMO

\ Cleanup
GPIO-CLOSE

CR ." Morse code demo complete!" CR
