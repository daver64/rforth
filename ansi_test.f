( ANSI Forth Compatibility Test )

1 2 3 drop swap . . cr

5 3 + . 10 4 - . 6 7 * . 20 4 / . cr

5 3 > . 3 5 < . 4 4 = . cr

variable test-var
42 test-var !
test-var @ . cr

: test-if dup 0 > if ." positive " then . ;
5 test-if cr

10 0 do i . loop cr

65 emit 66 emit 67 emit cr

15 7 3 */ . cr

s" Hello World" type cr

: square dup * ;
5 square . cr

." All tests completed!" cr