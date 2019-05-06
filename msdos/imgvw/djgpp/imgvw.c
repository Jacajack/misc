#include "imgvw.h"

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
		l &= 0b11000000;
		r = ( r >> 6 ) & 0b00000011;
		g = ( g >> 4 ) & 0b00001100;
		b = ( b >> 2 ) & 0b00110000;
		
		return l | r | g | b;
	}
	else //Standard 64 color mode
	{
		r = ( r & 0b11000000 ) >> 6;
		g = ( g & 0b11000000 ) >> 4;
		b = ( b & 0b11000000 ) >> 2; 
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
			r = ( i & 0b00000011 ) << 1;
			g = ( i & 0b00001100 ) >> 1;
			b = ( i & 0b00110000 ) >> 3;
			
			s = i >> 6 & 0b00000011;
			
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
		for ( i = 0; i < 0b00111111; i++ )
		{
			r = ( i & 0b00000011 ) << 4;
			g = ( i & 0b00001100 ) << 2;
			b = ( i & 0b00110000 ) << 0; 
			
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
	__djgpp_nearptr_enable( );
	VGA = (char*)( 0xA0000 + __djgpp_conventional_base );
	gfxMode( 0x13 );
	
	//Set up color pallete
	setupPallete( );
}

void gfxDisable( )
{
	//Go back to text mode, disable nearptr, close file - finish doing stuff
	gfxMode( 0x03 );
	__djgpp_nearptr_disable( );
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
		
	for ( i = 0; i < header.bmpHeaderSize - 16 && ( c = getc( image ) ) != EOF; i++ );
	
	//Error check
	if ( c == EOF )
		errExit( "Unexpected EOF reached!\n" );
	
	//Read width, height and calculate padding size
	width = header.width;
	height = header.height;
	padding = ( width + 4 ) % 4;
	
	//Check width, height and bit depth
	if ( width > GFX_WIDTH || height > GFX_HEIGHT )
		errExit( "Image too big!" );

	if ( header.bitDepth != 24 )
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
