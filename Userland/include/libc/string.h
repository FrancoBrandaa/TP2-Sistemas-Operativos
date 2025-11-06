#ifndef _LIBC_STRING_H_
#define _LIBC_STRING_H_

#include <stddef.h>
#include <stdint.h>

int strlen(const char * str);
int strcmp(const char * str1, const char * str2);
int strcasecmp(char * str1, char * str2);
void strcpy(char * dest, char * src);
void strncpy(char * dest, char * src, int n);
void perror(const char * s1);
char * strtok(char * s1, const char * s2);
char * strchr(const char * s, int c);
void * memmove(void * dest, const void * src, int n);
void * memset(void * destiation, int32_t c, uint64_t length);
int int_to_string(int num, char *str);

#endif