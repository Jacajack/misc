#include <stdio.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>

#define SAMPLE_RATE 8000.0

// Sample output
void outsample( float v )
{
	// Clamp
	if ( v > 1 ) v = 1;
	else if ( v < -1 ) v = -1;

	putchar( 255 * ( v + 1 ) / 2 );
}

// Sample output for int16_t data
void ioutsample( int32_t v )
{
	v += 32768;
	putchar( v >> 8 );
}

// Integer based sine
// This is to be replaced with ROM memory
int16_t isin( uint32_t x )
{
	uint16_t phi = x >> 20;
	return 32767 * sin( phi / 4096.0 * 2 * M_PI );
}

// Integer based two oscillator stack
int16_t iosc_stack2( uint32_t c_step )
{
	static uint32_t c_phase = 0; // Carrier phase
	static uint32_t m_phase = 0; // Modulator phase
	
	// Fixed-point params
	uint32_t mod_freq = ( 1 << 16 ) * M_PI; 
	
	double mod_amp = UINT32_MAX / 2 / M_PI * 0.2;

	int16_t modulator = isin( m_phase );
	int16_t carrier = isin( c_phase + mod_amp * modulator / 32768 ); 
	
	c_phase += c_step; // Increment phase
	m_phase += ( (uint64_t) c_step * mod_freq ) >> 16;
	
	return carrier;
}

// Floating point two oscillator stack
float osc_stack2( double t, float freq )
{
	double Ac = 1;
	double Am = 0.2;
	double I = M_PI;
	
	double wc = freq * 2 * M_PI;
	double wm = wc * I;	

	return Ac * sin( wc * t + Am * cos( wm * t ) );
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
		//outsample( osc_stack2( t, freq ) * envelope );		
		ioutsample( iosc_stack2( UINT32_MAX * freq / SAMPLE_RATE  ) * envelope );
		
		cnt++;
	}
	
	return 0;
}
