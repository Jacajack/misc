#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct __attribute__((packed)) Pilot
{
	char name[22];
	uint32_t score;
	uint16_t score2;
	uint16_t flags;
	uint16_t flags2;
} Pilot;

#define MAX_PILOTS  8
#define RANK_BITS   0x0f
#define FLAG_DEAD   0x2000
#define FLAG_OFFICE 0x4000

void print_pilot(const Pilot *p)
{
	printf(
		"Pilot:  %20s\n"
		"Score:  %d (%d)\n"
		"Flags:  %04x\n"
		"Rank:   %d\n"
		"Dead:   %s\n"
		"Office: %s\n"
		"  bis: %04x\n",
		p->name,
		(int) p->score,
		(int) p->score2,
		(int) p->flags,
		p->flags & RANK_BITS,
		p->flags & FLAG_DEAD ? "yes" : "no",
		p->flags & FLAG_OFFICE ? "yes" : "no",
		(int) p->flags2
	);
}

void list_pilots(Pilot pilots[static MAX_PILOTS])
{
	for (int i = 0; i < MAX_PILOTS; i++)
		printf("%d. '%20s'\n", i, pilots[i].name);
}

void read_file(const char *path, uint8_t **file_data, size_t *file_size)
{
	FILE *f = fopen(path, "rb");
	if (!f)
	{
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}
	
// 	printf("%ld\n", sizeof(Pilot));

	fseek(f, 0, SEEK_END);
	*file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	assert(*file_size == MAX_PILOTS * sizeof(Pilot) + 2);
	
	*file_data = calloc(*file_size, 1);
	assert(*file_data != NULL);
	fread(*file_data, *file_size, 1, f);
	fclose(f);
}

void write_file(const char *path, uint8_t *file_data, size_t file_size)
{
	FILE *f = fopen(path, "wb");
	fwrite(file_data, file_size, 1, f);
	fclose(f);
}

int cmd_list(Pilot *pilots, const char *argv[], int argc)
{
	if (argc != 0)
	{
		fprintf(stderr, "Usage: list\n");
		return 1;
	}
	
	list_pilots(pilots);
	
	return 0;
}

int cmd_resurrect(Pilot *pilots, const char *argv[], int argc)
{
	int id;
	if (argc != 1 || !sscanf(argv[0], "%d", &id))
	{
		fprintf(stderr, "Usage: resurrect <PILOT ID>\n");
		return 1;
	}
	
	if (id < 0 || id >= MAX_PILOTS)
	{
		fprintf(stderr, "Invalid Pilot ID!\n");
		return 1;
	}
	
	pilots[id].flags &= ~(FLAG_DEAD | FLAG_OFFICE);

	return 0;
}

int main(int argc, const char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s <HALLFAME> <COMMAND> [ARGS]\n", argv[0]);
		return 1;
	}
	
	const char *path = argv[1];
	const char *command = argv[2];
	
	// Read file
	uint8_t *file_data;
	size_t file_size;
	read_file(path, &file_data, &file_size);
	
	// Exctract pilot data
	Pilot pilots[MAX_PILOTS];
	memcpy(pilots, file_data + 2, sizeof(pilots));

	if (!strcmp(command, "list"))
		cmd_list(pilots, argv + 3, argc - 3);
	if (!strcmp(command, "resurrect"))
		cmd_resurrect(pilots, argv + 3, argc - 3);
	else
		fprintf(stderr, "Unknown command!\n");
	
	// Write back
	memcpy(file_data + 2, pilots, sizeof(pilots));
	write_file(path, file_data, file_size);
	free(file_data);
	return 0;
}
