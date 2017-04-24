#ifndef H_UTIL
#define H_UTIL

void line_perror(int line, const char* func_name);

#define my_perror() line_perror(__LINE__, __func__);
#endif	// H_UTIL
