#include <stdio.h>
#include "util.h"

void line_perror(int line) {
	char s[20];
	snprintf(s, 21, "l%d", line);
	perror(s);
};
