#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <argp.h>

// Global state and config
struct state
{
	const char *filename;
	int append;
	int flush_interval;
	int buffer_size;
	int should_flush;
	int active;
} state;

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
void alarm_handler( int signum )
{
	state.should_flush = 1;
	alarm( state.flush_interval );
}

// SIGINT handler for gentle exit
void sigint_handler( int signum )
{
	state.active = 0;
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

// Argp parser
const char *argp_program_version = "diskmercy v1.0";
const char *argp_program_bug_address = "<mrjjot@gmail.com>";
const char argp_doc[] = "diskmercy - abusive disk write restrainer";
const char argp_keydoc[] = "FILENAME";
const struct argp_option argp_options[] = 
{
	{"append", 'a', 0, 0, "Append data to output file"},
	{"buffer", 'b', 0, 0, "Set internal buffer size [b]"},
	{"interval", 't', 0, 0, "Set buffer flush interval [s]" },

	{0}
};
error_t parse_opt( int key, char *arg, struct argp_state *argp )
{
	switch ( key )
	{
		// Append
		case 'a':
			state.append = 1;
			break;
		
		// Buffer size
		case 'b':
			if ( sscanf( arg, "%d", &state.buffer_size ) != 1 )
				argp_error( argp, "invalid buffer size" );
			if ( state.buffer_size <= 0 )
				argp_error( argp, "buffer size must be positive" );
			break;
			
		// Flush interval
		case 'i':
			if ( sscanf( arg, "%d", &state.flush_interval ) != 1 )
				argp_error( argp, "invalid flush interval" );	
			if ( state.flush_interval <= 0 )
				argp_error( argp, "flush interval must be positive" );
			break;
				
		case ARGP_KEY_ARG:
			if ( argp->arg_num >= 1 ) argp_usage( argp );
			state.filename = arg;
			break;
			
		case ARGP_KEY_END:
			if ( argp->arg_num < 1 ) argp_usage( argp );
			break;
			
		default:
			return ARGP_ERR_UNKNOWN;
	}
	
	return 0;
}

int main( int argc, char **argv )
{
	// Defaults
	state.buffer_size = 1024 * 1024;
	state.flush_interval = 0;
	state.append = 0;
	state.should_flush = 0;

	// Parse options
	static struct argp argp = {argp_options, parse_opt, argp_keydoc, argp_doc};
	argp_parse( &argp, argc, argv, 0, 0, NULL );
	
	// Clear the file
	if ( !state.append )
	{
		FILE *f = fopen( argv[1], "wb" );
		if ( f == NULL )
			perror( "fopen() failed when clearing the file" );
		if ( fclose( f )  == EOF )
			perror( "fclose() failed when clearing the file" );
	}

	// Buffer setup
	struct buffer buf;
	buffer_init( &buf, state.buffer_size, state.filename );
	
	// Schedule the first alarm and setup signal handlers
	signal_setup( );
	alarm( state.flush_interval );
	
	// This has to be global, because we want nice exit with SIGINT
	state.active = 1;
	while ( state.active )
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
		if ( state.should_flush )
		{
			buffer_flush( &buf );
			state.should_flush = 0;
		}
	
	}
	
	// Flush + cleanup
	buffer_flush( &buf );
	buffer_destroy( &buf );
	return EXIT_SUCCESS;
}
