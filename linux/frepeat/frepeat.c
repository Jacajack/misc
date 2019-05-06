/*
	A crude utility to repeat file contents n times.
	As far as I know, there's no UNIX tool to that yet.

	2018, Jacek Wieczorek
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main( int argc, char **argv )
{
	if ( argc != 3 )
	{
		fprintf( stderr, "Usage: frepeat <FILE> <COUNT>\n" );
		exit( EXIT_FAILURE );
	}
	
	// Repetition count
	const char *countstr = argv[2];
	int repeat_count;
	if ( sscanf( countstr, "%d", &repeat_count ) != 1 )
	{
		fprintf( stderr, "Bad count!\n" );
		exit( EXIT_FAILURE );
	}

	// Input file
	const char *fname = argv[1];
	FILE *f = fopen( fname, "rb" );
	if ( f == NULL )
	{
		fprintf( stderr, "Cannot open file! (%s)\n", strerror( errno ) );
		exit( EXIT_FAILURE );
	}
	if ( fseek( f, 0, SEEK_END ) == -1 )
	{
		fprintf( stderr, "fseek() failed! (%s)\n", strerror( errno ) );
		exit( EXIT_FAILURE );
	}
	int input_len = ftell( f );
	if ( input_len == -1 )
	{
		fprintf( stderr, "ftell() failed! (%s)\n", strerror( errno ) );
		exit( EXIT_FAILURE );
	}
	if ( fseek( f, 0, SEEK_SET ) == -1 )
	{
		fprintf( stderr, "fseek() failed! (%s)\n", strerror( errno ) );
		exit( EXIT_FAILURE );
	}

	// Input buffer
	char *input = malloc( input_len );
	if ( input == NULL )
	{
		fprintf( stderr, "malloc() failed! (%s)\n", strerror( errno ) );
		exit( EXIT_FAILURE );
	}
	if ( fread( input, 1, input_len, f ) != input_len )
	{
		fprintf( stderr, "fread() short read/error! (%s)\n", strerror( errno ) );
		exit( EXIT_FAILURE );
	}
	fclose( f );


	// Output data
	int i, j;
	for ( i = 0; i < repeat_count; i++ )
	{
		for ( j = 0; j < input_len; j++ )
			putchar( input[j] );
	}

	return 0;
}
