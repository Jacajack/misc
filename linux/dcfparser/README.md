# dcfparser

DCF77 signal parser library + file parser.

<hr>

File parser usage:
`xzcat data/dcf5.txt.xz | ./dcfparser`

The parser file should contain following collumns (in that order):
 - UNIX timestamp
 - Time since logger startup (ms)
 - Input state duration (ms)
 - New input state (1/0)
 
