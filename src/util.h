#ifndef H_UTIL
#define H_UTIL

void line_perror(int line);

#define my_perror() line_perror(__LINE__);
#endif	// H_UTIL
