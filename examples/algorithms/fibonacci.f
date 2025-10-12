: add-one 1 + ;
: times-two 2 * ;
: process dup add-one times-two ;

10 process . 
5 process .
cr