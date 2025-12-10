\ Button Control LED Example
\ Hardware: 
\   - LED and 220 ohm resistor between GPIO17 and GND
\   - Button between GPIO27 and GND (uses internal pull-up)

\ Initialize GPIO
GPIO-INIT

\ Configure pins
17 GPIO-OUTPUT        \ LED output
27 GPIO-INPUT         \ Button input
27 GPIO-PULL-UP       \ Enable pull-up on button

\ Define button-led word
: BUTTON-LED
  ." Press button to control LED (Ctrl+C to exit)" CR
  BEGIN
    27 GPIO-READ      \ Read button state
    IF
      17 GPIO-CLR     \ Button pressed (LOW), LED off
    ELSE
      17 GPIO-SET     \ Button released (HIGH), LED on
    THEN
    50 DELAY-MS       \ Small delay for debouncing
  0 UNTIL ;           \ Loop forever (use Ctrl+C to exit)

\ Run the button control
BUTTON-LED

\ This won't be reached unless modified
GPIO-CLOSE
