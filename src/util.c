#include <stdio.h>
#include "util.h"

void line_perror(int line, const char* func_name) {
	char s[20];
	snprintf(s, 21, "%s:%d", func_name, line);
	perror(s);
};
