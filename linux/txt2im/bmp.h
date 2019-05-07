#ifndef BMP_H
#define BMP_H
#include <stdio.h>
#include <inttypes.h>

typedef union
{
	struct
	{
		uint8_t b, g, r;
	} __attribute__( ( packed ) );;
	uint32_t colnum;
} Pixel;


typedef struct
{
	union
	{
		struct
		{
			uint16_t fileType; 		//2b
			uint32_t fileSize;  	//4b
			uint16_t reserved0;   	//2b
			uint16_t reserved1; 	//2b
			uint32_t bmpOffset; 	//4b

			uint32_t bmpHeaderSize; //4b
			uint16_t width;  		//2b
			uint16_t height; 		//2b
			uint16_t planes; 		//2b
			uint16_t bitsPerPixel;	//2b
		} __attribute__( ( packed ) );

		char header[26];
	};
	Pixel **imageData;
} BMP2;

extern uint8_t bmpInit( BMP2 *bmp, uint16_t w, uint16_t h );
extern uint8_t bmpRead( BMP2 *bitmap, FILE *f );
extern void bmpFree( BMP2 *bmp );
extern uint8_t bmpWrite( BMP2 *bitmap, FILE *f );
extern Pixel pixrgb( uint8_t r, uint8_t g, uint8_t b );

#endif
