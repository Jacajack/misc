#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

const char *vowels = "euioay";
const char *consonants = "wrtpsdfghjklzxcvbnm";

int main( int argc, char **argv )
{
	int count, endless = 0;
	char *s;
	
	if ( argc < 3 )
	{
		printf( "Usage: %s COUNT TEMPLATE [MORE TEMPLATES]\n", argv[0] );
		return 1;
	}
	
	srand( time( NULL ) );
	
	if ( !sscanf( argv[1], "%d", &count ) )
	{
		fprintf( stderr, "%s: bad word count!\n", argv[0] );
		return 1;
	}
	
	if ( count < 1 ) 
	{
		endless = 1;
	}
	
	while ( count-- || endless )
	{
		s = argv[2 + rand( ) % ( argc - 2 )];
		while ( *s )
		{
			if ( toupper( *s ) == *s )
			{
				printf( "%c", tolower( *s ) );
			}
		
			if ( strchr( consonants, *s ) != NULL )
				printf( "%c", consonants[rand( ) % strlen( consonants )] );
			else if ( strchr( vowels, *s ) != NULL )
				printf( "%c", vowels[rand( ) % strlen( vowels )] );
			s++;
		}
		if ( endless ) getchar( );
		else printf( "\n" );
	}
}
