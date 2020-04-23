# mkwav

A utility for converting arbitrary 8-bit binary data into 16-bit, mono PCM WAVE files.
It esentially works just like `aplay` but saves to file instead of playing the sound.

Usage:
	`cat file.bin | mkmif <OUTPUT FILE> <SAMPLERATE>`
