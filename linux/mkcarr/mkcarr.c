#include <stdio.h>

int main( int argc, char **argv )
{
	// Array name
	const char *array_name = "data";
	if ( argc > 1 ) array_name = argv[1];
	
	// Preamble
	printf(
			"#include <inttypes.h>\n"
			"\n"
			"uint8_t %s[] = {\n",
			array_name );

	// The data
	int c, cnt = 0;
	while ( ( c = getchar( ) ) != EOF )
	{
		// The data
		printf( "\t0x%02x,", c );

		// Print offset every 16 bytes
		if ( cnt % 16 == 0 )
			printf( " // 0x%08x", cnt );

		putchar( '\n' );
		cnt++;
	}

	// The end
	printf( "};\n" );

	return 0;
}
