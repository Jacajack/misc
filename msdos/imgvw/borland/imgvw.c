#include <stdio.h>
#include <conio.h>
//#include <inttypes.h>
//#include <stdint.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/nearptr.h>
#include <dos.h>

#define VERSION "v0.2"
#define GFX_WIDTH 320
#define GFX_HEIGHT 200
#define FLAG_GRAYSCALE 1
#define FLAG_C256 2
#define FLAG_BMP3 4

unsigned char *VGA;

typedef unsigned char uint8_t;
typedef unsigned int  uint16_t;
typedef unsigned long uint32_t;

#if sizeof(uint16_t) != 2
#error bad uint16_t
#endif

#if sizeof(uint32_t) != 4
#error bad uint32_t
#endif


typedef union
{
	struct 
	{
		uint16_t fileType;
		uint32_t fileSize;
		uint16_t reserved0;
		uint16_t reserved1;
		uint32_t bmpOffset;
		
		uint32_t bmpHeaderSize;
		uint32_t width;
		uint32_t height;
		uint16_t planes;
		uint16_t bitDepth;
	} s; //packeds
	
	char raw[40];
	
} BMP;

uint16_t flags;

void gfxEnable( );
void gfxDisable( );


void gfxMode( char mode )
{
	//Set video mode
	union REGS regs;
	regs.h.ah = 0x00; //BIOS set video mode function
	regs.h.al = mode; //256-color mode
	int86( 0x10, &regs, &regs );
}

void errExit( const char *message )
{
	gfxDisable( );
	fprintf( stderr, "%s", message );
	exit( 1 );
}

char rgbToPallete( int r, int g, int b, int lightness )
{
	uint16_t i, l;
	
	if ( flags & FLAG_GRAYSCALE ) //Grayscale mode
	{
		r >>= 2;
		g >>= 2;
		b >>= 2;	
		return ( r + g + b ) / 3;
	}
	else if ( flags & FLAG_C256 ) //256 color mode
	{			
		l = ( r + g + b + lightness * 256 ) / ( 3 + lightness );
		l &= ( 3 << 6 ); //0b11000000;
		r = ( r >> 6 ) & 3; //0b00000011;
		g = ( g >> 4 ) & ( 3 << 2 ); //0b00001100;
		b = ( b >> 2 ) & ( 3 << 4 ); //0b00110000;
		
		return l | r | g | b;
	}
	else //Standard 64 color mode
	{
		r = ( r & ( 3 << 6 ) ) >> 6;
		g = ( g & ( 3 << 6 ) ) >> 4;
		b = ( b & ( 3 << 6 ) ) >> 2; 
		return r | g | b;
	}
}

void setupPallete( )
{
	int i, r, g, b, s;
	
	//Set color pallete	
	if ( flags & FLAG_GRAYSCALE )
	{
		// 64 shades of grey pallete
		outp( 0x03c8, 0 );
		for( i = 0; i < 256; i++ )
		{		
 			outp( 0x03c9, i );
  			outp( 0x03c9, i );
  			outp( 0x03c9, i );
		}
	
	}
	else if ( flags & FLAG_C256 ) //256 color pallete
	{
	
		outp( 0x03c8, 0 );
		for ( i = 0; i < 256; i++ )
		{
			r = ( i & ( 3 << 0 ) ) << 1;
			g = ( i & ( 3 << 2 ) ) >> 1;
			b = ( i & ( 3 << 4 ) ) >> 3;
			
			s = i >> 6 & 3;
			
			r <<= s;
			g <<= s;
			b <<= s;
			
 			outp( 0x03c9, r );
  			outp( 0x03c9, g );
  			outp( 0x03c9, b );
		}	
	}
	else //64 color pallete
	{
		for ( i = 0; i < 63; i++ )
		{
			r = ( i & ( 3 << 0 ) ) << 4;
			g = ( i & ( 3 << 2 ) ) << 2;
			b = ( i & ( 3 << 4 ) ) << 0; 
			
			outp( 0x03c8, i );
			outp( 0x03c9, r );
  			outp( 0x03c9, g );
  			outp( 0x03c9, b );
		}
	}
}	

void gfxEnable( )
{
	//Init nearptr and enable 256-color mode
	//__djgpp_nearptr_enable( );
	//VGA = (char*)( 0xA0000 + __djgpp_conventional_base );
	VGA = (unsigned char*)( 0xA0000000L );
	gfxMode( 0x13 );
	
	//Set up color pallete
	setupPallete( );
}

void gfxDisable( )
{
	//Go back to text mode, disable nearptr, close file - finish doing stuff
	gfxMode( 0x03 );
	//__djgpp_nearptr_disable( );
}

void help( )
{
	printf( "imgvw " VERSION "\n" );
	printf( "Usage: \n" );
	printf( "\t-g - grayscale mode\n" );
	printf( "\t-h - display this help message\n" );
	printf( "\t-l [number] - lightness (only with 256 color mode)\n" );
	printf( "\t-256 - 256 color mode\n" );
	printf( "\n\nNote: IMGVW supports only BMP3 files currently.\n" );
	exit( 0 );
}

int main( int argc, char **argv )
{
	int r, g, b, c, s, lightness = 4;
	uint16_t width, height, padding;
	uint32_t i, j, offset;
	FILE *image;
	BMP header;
	
	//No command line arguments
	if ( argc == 1 )
		errExit( "Please specify image filename!\n" );
	
	for ( i = 1; i < argc; i++ )
	{
		if ( !strcmp( argv[i], "-g" ) )
			flags |= FLAG_GRAYSCALE;
		
		if ( !strcmp( argv[i], "-256" ) )
			flags |= FLAG_C256;
		
		if ( !strcmp( argv[i], "-l" ) )
			lightness = atoi( argv[i + 1 > argc ? i : i + 1] );
		
		if ( !strcmp( argv[i], "-h" ) )
			help( );
	}
	

	if ( ( image = fopen( argv[argc - 1], "rb" ) ) == NULL )
		errExit( "Cannot open given file!\n" );

	//Read header	
	for ( i = 0; i < 30 && ( c = getc( image ) ) != EOF; i++ )
		header.raw[i] = c;
	
	//Error check
	if ( c == EOF )
		errExit( "Unexpected EOF reached!\n" );
		
	for ( i = 0; i < header.s.bmpHeaderSize - 16 && ( c = getc( image ) ) != EOF; i++ );
	
	//Error check
	if ( c == EOF )
		errExit( "Unexpected EOF reached!\n" );
	
	//Read width, height and calculate padding size
	width = header.s.width;
	height = header.s.height;
	padding = ( width + 4 ) % 4;
	
	//Check width, height and bit depth
	if ( width > GFX_WIDTH || height > GFX_HEIGHT )
		errExit( "Image too big!" );

	if ( header.s.bitDepth != 24 )
		errExit( "Only TrueColor images are supported!" );

	//Calculate offset to center the image
	offset = ( GFX_WIDTH - width ) / 2;
	offset += ( GFX_HEIGHT - height ) * GFX_WIDTH / 2; 

	gfxEnable( );
	
	
	//Write image to graphics memory in and read image in the same time (deal with it...)
	for ( i = 0; i < ( height > GFX_HEIGHT ? GFX_HEIGHT : height ); i++ )
	{
		for ( j = 0; j < ( width > GFX_WIDTH ? GFX_WIDTH : width ); j++ )
		{
			//Read color data
			( ( b = getc( image ) ) != EOF ) && ( ( g = getc( image ) ) != EOF ) && ( ( r = getc( image ) ) != EOF ); 
			if ( b == EOF || g == EOF || r == EOF )
				errExit( "Unexpected EOF reached!\n" );
			
			c = rgbToPallete( r, g, b, lightness );
			
			//Write color data to VGA memory
			VGA[(GFX_HEIGHT - i) * GFX_WIDTH + j + offset] = c;
		}
		
		//Ignore padding
		for ( j = 0; j < padding && i != height - 1; j++ )
			if ( getc( image ) == EOF ) errExit( "Unexpected EOF reached!\n" );
	}
	
	//Wait for keypress
	getch( );
	
	gfxDisable( );
	fclose( image );
	printf( "Did you like that image? Welcome back to DOS! :)\n" );
	return 0;
}
