#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_LENGTH 1024
#define COORDS_MAX 30
#define TOLERANCE 500 //in m
#define SCAN_FMT  "%lf,%lf,0"
#define PRINT_FMT "%lf, %lf, %d, %d\n"
struct radio_points {
	double lat;
	double lon;
};

int main(int argc, char *argv[]) {
	int i, a;
	FILE *fptr;
	FILE *fout;
	char line_buffer[BUFFER_LENGTH];
	char token_buffer[BUFFER_LENGTH];
	struct radio_points data;

	printf("Executable: %s\n", argv[0]);

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			printf("argv[%d]: %s\n", i, argv[i]);
		}
	} else {
		printf("No arguments were provided.\n");
	}

	if (argc != 2) {
		printf("usage: kml-parser.exe filename.kml\n");
		exit(EXIT_FAILURE);
	}

	fptr = fopen(argv[1], "r");
	fout = fopen("send_data.txt","w");

	if (fptr == NULL) {
		printf("File %s could not be opened!\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	while (fgets(line_buffer, BUFFER_LENGTH, fptr) != NULL) {
		if (sscanf(line_buffer, "%s", token_buffer) != 1) {
			continue;
		}
		if (strcmp(token_buffer, "<coordinates>") != 0) {
			continue;
		}

		printf("%s", line_buffer);

		for (a = 0; a <= COORDS_MAX; a++) {
			if (fscanf(fptr, "%lf,%lf,0 ", &data.lon, &data.lat) != 2) {
				printf("Could not retrieve coordinates");
				break;
			}
			int next_idx = a+1;
			fprintf(fout, PRINT_FMT,data.lat,data.lon,TOLERANCE,next_idx);
		}
		break;
	}

	if (feof(fptr)) {
		puts("End of file reached.\n");
	}

	fclose(fptr);
	fclose(fout);
	exit(EXIT_SUCCESS);

}
