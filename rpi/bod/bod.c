#include <stdio.h>
#include <wiringPi.h>
#include <time.h>

#define VCC_PIN 4
#define INP_PIN 5 

void bodlog( FILE *f, unsigned long ms )
{
	fprintf( f, "%lu of %lu ms\n", time( NULL ), ms );
}

int main( int argc, char **argv )
{
	wiringPiSetup( );
	pinMode( VCC_PIN, OUTPUT );
	pinMode( INP_PIN, INPUT );
	digitalWrite( VCC_PIN, 1 );

	unsigned long ms;
	int last = 0;

	while ( 1 )
	{
		// Wait for edge
		for ( ms = 0; last == digitalRead( INP_PIN ); ms++ )
		{
			// Sleep 1ms
			struct timespec ts = {
				.tv_sec = 0,
				.tv_nsec = 1e6
			};
			nanosleep( &ts, NULL );
		}
		last = !last;

		// Log
		if ( ms > 15 )
		{
			bodlog( stderr, ms );
			if ( argv[1] != NULL )
			{
				FILE *f = fopen( argv[1], "a" );
				if ( f == NULL ) perror( "could not open log file!" );
				bodlog( f, ms );
				if ( fclose( f ) ) perror( "could not close log file!" );
			}
		}
	}
}
