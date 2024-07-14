#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

// https://graphics.stanford.edu/~seander/bithacks.html
unsigned int interleave_bits(unsigned int x, unsigned int y)
{
	static const unsigned int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
	static const unsigned int S[] = {1, 2, 4, 8};

	x = (x | (x << S[3])) & B[3];
	x = (x | (x << S[2])) & B[2];
	x = (x | (x << S[1])) & B[1];
	x = (x | (x << S[0])) & B[0];

	y = (y | (y << S[3])) & B[3];
	y = (y | (y << S[2])) & B[2];
	y = (y | (y << S[1])) & B[1];
	y = (y | (y << S[0])) & B[0];

	return x | (y << 1);
}

// https://graphics.stanford.edu/~seander/bithacks.html
unsigned int reverse_bits(unsigned int v)
{
	// swap odd and even bits
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ...
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = ( v >> 16             ) | ( v               << 16);
	return v;
}

// Broken for n >= 8
int bayer_matrix(int n, int x, int y)
{
	int size = 1 << n;
	/*
	static const int mat_4[4][4] = {
		{0, 8, 2, 10},
		{12, 4, 14, 6},
		{3, 11, 1, 9},
		{15, 7, 13, 5}
	};

	static const int mat_2[2][2] = {
		{0, 2},
		{3, 1},
	};
	*/

// 	if (n == 2) return mat_2[x][y] - 2;
// 	else if (n == 4) return mat_4[x][y] - 8;
// 	else
		return (reverse_bits(interleave_bits(x ^ y, y)) >> (32 - 2 * n)) - size * size / 2;
}

void dither_channel(uint8_t *img, int width, int height, int stride, int output_bpc, int mat_n)
{
// 	static const int output_bpc = 2;
	const int lost_bpc = 8 - output_bpc;

// 	static const int mat_n = 4;
    const int mat_size = 1 << mat_n;

    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
        {
            uint8_t *c = &img[(x + y * width) * stride];

			int bias = bayer_matrix(mat_n, x % mat_size, y % mat_size);
			int shift = lost_bpc + 1 - 2 * mat_n;
			int biased_pixel = *c + (shift >= 0 ? (bias << shift) : (bias >> -shift));

			if (biased_pixel < 0)
				biased_pixel = 0;
			else if (biased_pixel > 255)
				biased_pixel = 255;

            *c = biased_pixel & (0xff << (8 - output_bpc));

			// Compensate brightness loss
			int tmp = *c;
			for (int i = 1; i < (lost_bpc + output_bpc + 1) / output_bpc; i++)
				tmp |= (*c >> (i * output_bpc));
			*c = tmp;

        }
}

void dither_image(uint8_t *img, int width, int height, int channels, int output_bpc, int mat_n)
{
    for (int i = 0; i < channels; i++)
        dither_channel(img + i, width, height, channels, output_bpc, mat_n);
}

void test(int n)
{
	int size = 1 << n;
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
			printf("%5d", bayer_matrix(n, x, y));
		printf("\n");
	}
}

int main(int argc, char *argv[])
{
    assert(argc == 5);

	// Usage: infile outfile bpc degree

    int width, height, channels;
    uint8_t *img = stbi_load(argv[1], &width, &height, &channels, 0);
    assert(img != NULL);

	int output_bpc = 2, mat_n = 2;
	sscanf(argv[3], "%d", &output_bpc);
	sscanf(argv[4], "%d", &mat_n);

	dither_image(img, width, height, channels, output_bpc, mat_n);

    int ok = stbi_write_png(argv[2], width, height, channels, img, width * channels);
    assert(ok);

// 	printf("0x%08x\n", interleave_bits(2, 0));
	printf("0x%08x\n", interleave_bits(0, 2));
// 	printf("0x%08x\n", reverse_bits(interleave_bits(2, 0)));
	printf("0x%08x\n", reverse_bits(interleave_bits(0, 2)));
	// printf("0x%08x\n", reverse_bits(interleave_bits(2, 0)) >> (32 - 8));
	printf("0x%08x\n", reverse_bits(interleave_bits(0, 2)) >> (32 - 8));

	test(1);
	printf("----------------\n");
	test(2);
	printf("----------------\n");
	test(3);


    return 0;
}
