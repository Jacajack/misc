#include <stdio.h>
#include <math.h>
#include <time.h>

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
	double dt = 1.0 / SAMPLE_RATE;
	
	double tcarrier = 0;
	float envelope = 0;
	
	while ( 1 )
	{
		t = cnt / SAMPLE_RATE;
		
		// Branch FM algorithm
		tcarrier += dt \
			+ 0.0011 * sinewave( t, 1000 )
			+ 0.0010 * sinewave( t, 1002 );
		
		// Carrier frequency
		float carrier = sinewave( tcarrier, 110 );
		
		// Simple envelope
		if ( cnt % (long)( SAMPLE_RATE * 3 ) == 0 ) envelope = 1;
		else if ( envelope <= 0 ) envelope = 0;
		else envelope -= 0.75 / SAMPLE_RATE;
		
		// Output the sample	
		outsample( carrier * envelope );		
		
		cnt++;
	}
	
	return 0;
}
