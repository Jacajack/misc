#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

static uint8_t bmpAlloc( BMP2 *bmp, uint16_t w, uint16_t h )
{
	uint16_t i;

	if ( ( bmp->imageData = (Pixel**) calloc( w, sizeof( Pixel* ) ) ) == NULL )
 		return 1;
	for ( i = 0; i < w; i++ )
		if ( ( bmp->imageData[i] = (Pixel*) calloc( h, sizeof( Pixel ) ) ) == NULL )
			return 1;

	return 0;
}

uint8_t bmpInit( BMP2 *bmp, uint16_t w, uint16_t h )
{
	bmp->fileType = 0x4D42;
	bmp->fileSize =  w * h * 3 + h * ( w % 4 ) + 26; //Image data (24bpx) + padding + header
	bmp->reserved0 = 0;
	bmp->reserved1 = 0;
	bmp->bmpOffset = 26;

	bmp->bmpHeaderSize = 12;
	bmp->width = w;
	bmp->height = h;
	bmp->planes = 1;
	bmp->bitsPerPixel = 24;

	return bmpAlloc( bmp, w, h );
}

void bmpFree( BMP2 *bmp )
{
	uint16_t i;

	for ( i = 0; i < bmp->width; i++ )
		 free ( bmp->imageData[i] );
	free(  bmp->imageData );
}

uint8_t bmpWrite( BMP2 *bmp, FILE *f )
{
	int i, j;

	for ( i = 0; i < 26; i++ )
		if ( fputc( bmp->header[i], f ) == EOF ) return 1;

	for ( i = bmp->height - 1; i >= 0; i-- )
	{
		for ( j = 0; j < bmp->width; j++ )
		{
			if ( fputc( bmp->imageData[j][i].b, f ) == EOF ) return 1;
			if ( fputc( bmp->imageData[j][i].g, f ) == EOF ) return 1;
			if ( fputc( bmp->imageData[j][i].r, f ) == EOF ) return 1;
		}

		for ( j = 0; j < bmp->width % 4; j++ )
			if ( fputc( 0, f ) == EOF ) return 1;
	}
	return 0;
}


uint8_t bmpRead( BMP2 *bmp, FILE *f )
{
	int i, j;

	for ( i = 0; i < 26; i++ )
		if ( ( bmp->header[i] = fgetc( f ) ) == EOF ) return 1;

	bmpAlloc( bmp, bmp->width, bmp->height );

	for ( i = bmp->height - 1; i >= 0; i-- )
	{
		for ( j = 0; j < bmp->width; j++ )
		{
			if ( ( bmp->imageData[j][i].b = fgetc( f ) ) == EOF ) return 1;
			if ( ( bmp->imageData[j][i].g = fgetc( f ) ) == EOF ) return 1;
			if ( ( bmp->imageData[j][i].r = fgetc( f ) ) == EOF ) return 1;
		}

		for ( j = 0; j < bmp->width % 4; j++ )
			if ( fgetc( f ) == EOF ) return 1;
	}
	return 0;
}

Pixel pixrgb( uint8_t r, uint8_t g, uint8_t b )
{
	Pixel pix;
	pix.r = r;
	pix.g = g;
	pix.b = b;
	return pix;
}
