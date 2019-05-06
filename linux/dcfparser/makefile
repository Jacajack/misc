all:
	clang -g -o dcfparser dcfparser.c dcf.c -Wall
	
test-avr-size:
	avr-gcc -c -o dcf-avr.o dcf.c -mmcu=attiny2313 -Os -DNDEBUG
	avr-size -C dcf-avr.o --mcu=attiny2313
