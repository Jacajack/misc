#include "dcf.h"
#include <time.h>
#include <inttypes.h>

// 8-bit BCD decoder
// Requires n > 0
static uint8_t dcf_decode_bcd( uint8_t *frame, uint8_t start, uint8_t n )
{
	uint8_t val = 0;
	uint8_t b = 1;
	uint8_t *end;
	
	frame += start;
	end = frame + n;

	// Spared 50 bytes with bitwise tricks
	do
	{
		val += *frame++ * b;
		if ( ( b <<= 1 ) == 16 ) b = 10;
	}
	while ( frame < end );
	
	return val;
}

// Checks parity of `n' bits starting at `start'
static uint8_t dcf_parity( uint8_t *frame, uint8_t start, uint8_t n )
{
	uint8_t parity = 0;
	uint8_t *end;
	
	frame += start;
	end = frame + n;
	
	while ( frame < end )
		parity ^= *frame++;
	
	return parity;
}

// Convert DCF77 frame to `struct tm`
static uint8_t dcf_parse( struct tm *t, uint8_t *frame )
{
	if ( t == NULL ) return 0;

	// Validate frame
	if ( frame[0] ) return 0;   // Start of minute - bit0 shall be 0
	if ( !frame[20] ) return 0; // Start of time - bit20 shall be 1
	if ( frame[17] == frame[18] ) return 0;      // Exclusive CET / CEST
	if ( dcf_parity( frame, 21, 8 ) ) return 0;  // Minute parity
	if ( dcf_parity( frame, 29, 7 ) ) return 0;  // Hour parity
	if ( dcf_parity( frame, 36, 23 ) ) return 0; // Date parity
	
	// Fill time struct
	t->tm_sec   = 0;
	t->tm_min   = dcf_decode_bcd( frame, 21, 7 );       // 7 bits - minute
	t->tm_hour  = dcf_decode_bcd( frame, 29, 6 );       // 6 bits - hour
	t->tm_mday  = dcf_decode_bcd( frame, 36, 6 );       // 6 bits - day of month
	t->tm_mon   = dcf_decode_bcd( frame, 45, 5 ) - 1;   // 5 bits - month
	t->tm_year  = dcf_decode_bcd( frame, 50, 8 ) + 100; // 8 bits - year
	t->tm_wday  = dcf_decode_bcd( frame, 42, 3 ) - 1;   // 3 bits - day of week
	t->tm_yday  = 0;          // FIXME?
	t->tm_isdst = frame[17];  // Daylight saving bit
	
	return 1;
}

// Enqueues an incoming impulse for parsing
uint8_t dcf_push_impulse( struct tm *t, uint8_t state, uint16_t duration )
{
	static uint8_t frame[59];  // Frame buffer
	static uint8_t length = 0; // Number of bits received so far
	uint8_t bit = 0; // Received bit value. Ignore if `err' is set
	uint8_t err = 0; // Set if impulse has invalid duration
	
	// Determine bit value
	if ( !state )
	{
		if ( duration < 600 || duration > 1400 ) err = 1;
	}
	else
	{
		if ( duration >= 40 && duration <= 130 ) bit = 0;       // 0 - 100ms
		else if ( duration >= 140 && duration <= 250 ) bit = 1; // 1 - 200ms
		else err = 1;
	}
		
	// On invalid impulse / length overflow
	if ( err || length == 59 )
	{
		err = 0;
		
		// Full frame + valid minute mark - attempt parsing
		if ( length == 59 && !state && duration > 1000 ) 
			err = dcf_parse( t, frame );
		
		length = 0;
		return err;
	}
	else if ( state == 1 )
		frame[length++] = bit;
		
	return 0;
}
