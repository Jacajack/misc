/*
	Utility to create *.mif (Memory Initialization File) for Quartus.
	Usage:
		cat file.bin | mkmif <WIDTH> <COUNT> > out.mif

	by Jacek Wieczorek, 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


// Cool help message
void show_usage( )
{
	fprintf( stderr, "Usage:\n\tcat file.bin | mkmif <WIDTH> <COUNT> > out.mif\n" );
}

int main( int argc, char **argv )
{
	// Check arg count
	if ( argc != 3 )
	{
		fprintf( stderr, argc < 3 ? "mkmif: missing arguments!\n" : "mkmif: too many arguments!\n" );
		show_usage( );
		exit( EXIT_FAILURE );
	}
	
	// Basic idiotproof level
	int wsize, count;
	if ( !sscanf( argv[1], "%d", &wsize ) || !sscanf( argv[2], "%d", &count ) || wsize <= 0 || count <= 0 )
	{
		fprintf( stderr, "mkmif: please provide valid integer parameters!\n" );
		show_usage( );
		exit( EXIT_FAILURE );
	}

	// I'm sorry
	if ( wsize % 8 != 0 || wsize > 64 )
	{
		fprintf( stderr, "mkmif: only word sizes up to 64 and divisible by 8 are supported... sorry!\n" );
		show_usage( );
		exit( EXIT_FAILURE );
	}

	// Prologue
	printf(
		"DEPTH = %d;\n" \
		"WIDTH = %d;\n" \
		"ADDRESS_RADIX = HEX;\n" \
		"DATA_RADIX = HEX;\n" \
		"CONTENT\n" \
		"BEGIN\n\n",
		count, wsize
	);
	
	union
	{
		uint8_t bytes[8];
		uint64_t word;
	} data = {0};
	
	int c, byteord = 0, address = 0;
	while ( ( c = getchar( ) ) != EOF && address < count )
	{
		data.bytes[byteord++] = c;
	
		if ( byteord == wsize / 8 )
		{
			printf( "%08X : %" PRIx64 ";\n", address, data.word );
			address++;
			byteord = 0;
		}
	} 
	
	// Epilogue
	printf( "\nEND;\n" );
}
