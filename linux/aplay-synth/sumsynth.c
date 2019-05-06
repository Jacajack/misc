#include <stdio.h>
#include <math.h>

#define SAMPLE_RATE 8000.0

// Sample output
void outsample( float v )
{
	// Clamp
	if ( v > 1 ) v = 1;
	else if ( v < -1 ) v = -1;

	putchar( 255 * ( v + 1 ) / 2 );
}

// Single harmonic function
float sinewave( double t, float freq )
{
	return sin( t * freq * M_PI * 2 );
}

int main( )
{
	long cnt = 0;
	double t = 0;
	
	#define HARMONICS 9
	float mix[HARMONICS] =
	{
		1,
		2,
		0,
		-2,
		-1,
		1,
		0,
		0.5,
		0.25,
	};
	
	while ( 1 )
	{
		t = cnt / SAMPLE_RATE;
	
		float buf = 0;
		for ( int i = 0; i < HARMONICS; i++ )
			buf += sinewave( t, 110 * ( i + 1 ) ) * mix[i];
		buf /= HARMONICS;
		
		outsample( buf );		
		
		cnt++;
	}
	
	return 0;
}
