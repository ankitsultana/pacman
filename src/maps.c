#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 300
#define HASH_SIZE 1300

int main() {
	char *grid = (char*)malloc(sizeof(char) * HASH_SIZE);
	char *buf = (char*)malloc(sizeof(char) * BUF_SIZE);
	int NUM_ROWS = 0, NUM_COLS = -1;
	int iter = 0, mul = -1, i;
#ifdef WYSIWYG
	mul = 2;
#else
	mul = 1;
#endif
	while(true) {
		if(fgets(buf, BUF_SIZE, stdin) == NULL) break;
		if(NUM_ROWS) {
			assert(mul*NUM_COLS == strlen(buf) - 1);
		} else {
			NUM_COLS = (strlen(buf) - 1) / mul;
		}
		int c;
		for(c = 0; c < NUM_COLS; c += mul) {
			grid[iter++] = buf[c];
			if(buf[c] != ' ' && buf[c] != '\n' && buf[c] != '*' && buf[c] != '.' &&
					buf[c] != 'X') {
				assert(false);
			}
		}
		grid[iter++] = '\n';
		NUM_ROWS++;
	}
	grid[iter++] = '\n';
	char *to_send = (char*)malloc(sizeof(char) * iter);
	for(i = 0; i < iter; i++) {
		to_send[i] = grid[i];
		//printf("%c", to_send[i]);
	}
	// Send data to client
	return 0;
}

#undef BUF_SIZE
#undef HASH_SIZE
#ifdef WYSIWYG
	#undef WYSIWYG
#endif
