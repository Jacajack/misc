#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "txt2im.h"
#include "bmp.h"

#define ASSOC_COUNT 256
Pixel assoc[ASSOC_COUNT] = {0};

static void measure( FILE *f, uint16_t *x, uint16_t *y )
{
	int c;
	uint16_t cx = 0;
	*x = *y = 0;
	fseek( f, SEEK_SET, 0 );
	while ( ( c = fgetc( f ) ) != EOF )
	{
		if ( c == '\n' )
		{
			cx = 0;
			( *y )++;
		}
		else if ( ++cx > *x ) *x = cx;
	}
}

static void txt2im( FILE *f, BMP2 *bmp )
{
	int c;
	uint16_t x = 0, y = 0;
	fseek( f, SEEK_SET, 0 );
	while ( ( c = fgetc( f ) ) != EOF )
	{
		if ( c == '\n' || c == '\r' )
		{
			x = 0;
			y++;
		}
		else
		{
			if ( x < bmp->width && y < bmp->height ) bmp->imageData[x++][y] = assoc[(uint8_t)c];
		}
	}
}

int main( int argc, char **argv )
{
	int i;
	uint16_t x, y;
	uint8_t badarg, id;
	uint32_t colnum;
	FILE *fi, *fo;
	char *finame = NULL, *foname = NULL;
	BMP2 bmp;

	if ( argc < 2 )
	{
		fprintf( stderr, "%s: too few arguments - use -h to see help\n", argv[0] );
		exit( 1 );
	}

	for ( i = 1; i < argc; i++ )
	{
		badarg = 1;

		//Help message
		if ( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "--help" ) )
		{
			fprintf( stderr, "%s - text map to image converter " TXT2IM_VERSION "\n", argv[0] );
			fprintf( stderr, "\t-h - display this help message\n" );
			fprintf( stderr, "\t-i <filename> - input file\n" );
			fprintf( stderr, "\t-o <filename> - output file\n" );
			fprintf( stderr, "\t-c <hex ASCII code>:<6-digit hex color> - assign color to character\n" );
			fprintf( stderr, "\t-C <character>:<6-digit hex color> - assign color to character\n" );

			badarg = 0;
			exit( 0 );
		}

		if ( !strcmp( argv[i], "-i" ) )
			if ( ++i >= argc || ( finame = argv[i], badarg = 0 ) ) fprintf( stderr, "%s: bad value for '%s'\n", argv[0], argv[i - 1] ), exit( 1 );

		if ( !strcmp( argv[i], "-o" ) )
			if ( ++i >= argc || ( foname = argv[i], badarg = 0 ) ) fprintf( stderr, "%s: bad value for '%s'\n", argv[0], argv[i - 1] ), exit( 1 );

		if ( !strcmp( argv[i], "-c" ) )
		{
			if ( ++i >= argc || ( badarg = 0, !sscanf( argv[i], "%hhx:%06x", &id, &colnum ) ) ) fprintf( stderr, "%s: bad value for '%s'\n", argv[0], argv[i - 1] ), exit( 1 );
			assoc[id].colnum = colnum;
		}

		if ( !strcmp( argv[i], "-C" ) )
		{
			if ( ++i >= argc || ( badarg = 0, !sscanf( argv[i], "%c:%06x", &id, &colnum ) ) ) fprintf( stderr, "%s: bad value for '%s'\n", argv[0], argv[i - 1] ), exit( 1 );
			assoc[id].colnum = colnum;
		}

		if ( badarg )
		{
			fprintf( stderr, "%s: unrecognized option '%s'\n", argv[0], argv[i] );
			exit( 1 );
		}

	}

	if ( finame == NULL )
	{
		fprintf( stderr, "%s: missing input file name\n", argv[0] );
		exit( 1 );
	}

	if ( foname == NULL )
	{
		fprintf( stderr, "%s: missing output file name\n", argv[0] );
		exit( 1 );
	}

	if ( ( fi = fopen( finame, "r" ) ) == NULL )
	{
		fprintf( stderr, "%s: cannot open %s\n", argv[0], finame );
		exit( 1 );
	}

	if ( ( fo = fopen( foname, "w" ) ) == NULL )
	{
		fprintf( stderr, "%s: cannot open %s\n", argv[0], foname );
		exit( 1 );
	}

	measure( fi, &x, &y );

	if ( x == 0 || y == 0 )
	{
		fprintf( stderr, "%s: input file dimensions too small\n", argv[0] );
		exit( 1 );
	}

	if ( bmpInit( &bmp, x, y ) )
	{
		fprintf( stderr, "%s: could not create bitmap\n", argv[0] );
		exit( 1 );
	}

	txt2im( fi, &bmp );

	if ( bmpWrite( &bmp, fo ) )
	{
		fprintf( stderr, "%s: could not write to output file\n", argv[0] );
		exit( 1 );
	}

	bmpFree( &bmp );
	fclose( fi );
	fclose( fo );
}
