#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

struct buffer
{
	uint8_t *data;
	size_t size;
	size_t written;
	char *filename;
};

// Flushes buffered data to file
void buffer_flush( struct buffer *buf )
{
	if ( buf->written == 0 ) return;
	
	FILE *f = fopen( buf->filename, "ab" );
	if ( f == NULL )
	{
		perror( "fopen() in buffer_flush() failed" );
		exit( EXIT_FAILURE );
	}
	
	if ( fwrite( buf->data, 1, buf->written, f ) < buf->written )
	{
		perror( "fwrite() in buffer_flush() failed" );
		fprintf( stderr, "fwrite() failed\n" );
		exit( EXIT_FAILURE );
	}
	
	if ( fclose( f ) != 0 )
	{
		perror( "fclose() in buffer_flush() failed" );
		exit( EXIT_FAILURE );
	}
	
	// Reset write counter
	buf->written = 0;
}

// Buffer data and flush if necessary
void buffer_push( struct buffer *buf, uint8_t b )
{
	// Flush if necessary
	if ( buf->written == buf->size ) buffer_flush( buf );
	
	// Store data
	buf->data[buf->written++] = b;
}

// Initialize buffer for use
void buffer_init( struct buffer *buf, size_t size, const char *filename )
{
	buf->size = size;
	buf->written = 0;
	buf->data = malloc( size );
	if ( buf->data == NULL )
	{
		perror( "malloc() in buffer_init() failed" );
		exit( EXIT_FAILURE );
	}
	
	buf->filename = strdup( filename );
	if ( buf->filename == NULL )
	{
		perror( "strdup() in buffer_init() failed" );
		exit( EXIT_FAILURE );
	}
}

// Clean up buffer after use
void buffer_destroy( struct buffer *buf )
{
	free( buf->data );
	free( buf->filename );
	buf->data = NULL;
	buf->size = 0;
	buf->written = 0;
} 

// Alarm handler + its global variables
int flush_interval = 0;
int should_flush = 0;
void alarm_handler( int signum )
{
	should_flush = 1;
	alarm( flush_interval );
}

// SIGINT handler for gentle exit
int active;
void sigint_handler( int signum )
{
	active = 0;
}

void signal_setup( )
{
	struct sigaction act;
	sigset_t mask;
	sigemptyset( &mask );
	sigaddset( &mask, SIGINT );
	sigaddset( &mask, SIGALRM );

	act.sa_handler = alarm_handler;
	act.sa_mask = mask;
	act.sa_flags = 0;
	if ( sigaction( SIGALRM, &act, NULL ) == -1 )
		perror( "sigaction() for SIGALRM failed" );
	
	act.sa_handler = sigint_handler;
	act.sa_mask = mask;
	act.sa_flags = 0;
	if ( sigaction( SIGINT, &act, NULL ) == -1 )
		perror( "sigaction() for SIGINT failed" );
}

int main( int argc, char **argv )
{
	if ( argc < 2 || argc > 4 )
	{
		fprintf( stderr, "Usage:\n\t%s FILE [buffer size] [flush interval]\n\nUse negative flush interval for no auto flushing.\n", argv[0] );
		exit( EXIT_FAILURE );
	}
	
	// The defaults - 1M and no auto-flush (global, because we have interrupts)
	int buffer_size = 1024 * 1024;
	flush_interval = 0;
	
	// Buffer size specified
	if ( argc > 2 )
	{
		if ( sscanf( argv[2], "%d", &buffer_size ) != 1 || buffer_size <= 0  )
		{
			fprintf( stderr, "Buffer size must be a positive integer!\n" );
			exit( EXIT_FAILURE );
		}
	}
	
	// Flush interval specified
	if ( argc > 3 )
	{
		if ( sscanf( argv[3], "%d", &flush_interval ) != 1 )
		{
			fprintf( stderr, "Flush interval must be an integer!\n" );
			exit( EXIT_FAILURE );
		}
		
		// Clamp to 0
		if ( flush_interval < 0 )
			flush_interval = 0;
	}
	
	// Clear the file
	FILE *f = fopen( argv[1], "wb" );
	if ( f == NULL )
		perror( "fopen() failed when clearing the file" );
	if ( fclose( f )  == EOF )
		perror( "fclose() failed when clearing the file" );

	// Buffer setup
	struct buffer buf;
	buffer_init( &buf, buffer_size, argv[1] );
	
	// Schedule the first alarm and setup signal handlers
	signal_setup( );
	alarm( flush_interval );
	
	// This has to be global, because we want nice exit with SIGINT
	active = 1;
	while ( active )
	{
		// Read from stdin
		// Using read() here, because it handles signals nicely
		uint8_t b;
		int ec = read( STDIN_FILENO, &b, 1 );
			
		if ( ec == 0 ) // Break on EOF
		{
			break;
		}
		else if ( ec == -1 ) // Handle errors and signals
		{
			// Only signals are acceptable
			if ( errno != EINTR )
			{
				perror( "read() from stdin failed" );
				break;
			}
		}
		else // Buffer data
		{
			buffer_push( &buf, b );
		}

		// Flushing
		if ( should_flush )
		{
			buffer_flush( &buf );
			should_flush = 0;
		}
	
	}
	
	// Flush + cleanup
	buffer_flush( &buf );
	buffer_destroy( &buf );
	return EXIT_SUCCESS;
}
