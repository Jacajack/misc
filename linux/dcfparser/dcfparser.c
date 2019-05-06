#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "dcf.h"

// A lovely debug utility
void dump64b( uint64_t val )
{
	uint64_t b;
	for ( b = 1ul << 63; b; b >>= 1 )
		putchar( val & b ? '1' : '0' );
	putchar( '\n' );
}

int main( )
{
	long timestamp;
	float runtime, duration;
	int state, success;
	int lines = 0, dates = 0;
	struct tm t;
	
	while ( scanf( "%ld %f %f %d", &timestamp, &runtime, &duration, &state ) == 4 )
	{
		success = dcf_push_impulse( &t, !state, duration );
		if ( success )
		{
			time_t tval = mktime( &t );
			printf( "------ %sUNIX: %ld\n DCF: %ld\nDIFF: %ld\nELIN: %d\n\n", asctime( &t ), timestamp, tval, labs( timestamp - tval ), lines );
			dates++;
		}
		
		lines++;
	}	
	
	fprintf( stderr, "Parsed %d lines and %d dates\n", lines, dates );
	
	return 0;	
}
