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

float osc_stack2( double t, float freq )
{
	double Ac = 1;
	double Am = 0.2;
	double I = 160;
	
	double wc = freq * 2 * M_PI;
	double wm = wc * I;	

	return Ac * sin( wc * t + Am * sin( wm * t  ) );
}

float osc_stack3( double t, float freq )
{
	double Ac = 1;
	double Am1 = 2;
	double Am2 = 2;
	double I1 = 2;
	double I2 = 0.1;
	
	double wc = freq * 2 * M_PI;
	double wm1 = wc * I1;
	double wm2 = wm1 * I2;	

	return Ac * sin( wc * t + Am1 * sin( wm1 * t + Am2 * sin( wm2 * t ) ) );
}

int main( )
{
	long cnt = 0;
	double t = 0;
	float envelope = 0;
	float freq = 110;
	
	
	while ( 1 )
	{
		t = cnt / SAMPLE_RATE;
		
		// Simple envelope
		if ( cnt % (long)( SAMPLE_RATE * 2 ) == 0 ) envelope = 1, freq += 110;
		else if ( envelope <= 0 ) envelope = 0;
		else envelope -= 0.75 / SAMPLE_RATE;
		
		// Output the sample	
		outsample( osc_stack2( t, freq ) * envelope );		
		
		cnt++;
	}
	
	return 0;
}
