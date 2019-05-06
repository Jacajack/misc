#include <stdio.h>
#include <conio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <sys/nearptr.h>
#include <dos.h>

#define VERSION "v0.2"
#define GFX_WIDTH 320
#define GFX_HEIGHT 200
#define FLAG_GRAYSCALE 1
#define FLAG_C256 2
#define FLAG_BMP3 4

char *VGA;

typedef union
{
	struct 
	{
		uint16_t fileType;
		uint32_t fileSize;
		uint16_t reserved0;
		uint16_t reserved1;
		uint32_t bmpOffset;
		
		uint32_t bmpHeaderSize;
		uint32_t width;
		uint32_t height;
		uint16_t planes;
		uint16_t bitDepth;
	} __attribute__( ( packed ) );
	
	char raw[40];
	
} BMP;

uint16_t flags;

void gfxEnable( );
void gfxDisable( );
