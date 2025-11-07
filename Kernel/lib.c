// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <stddef.h>
#include <lib.h>

void *memset(void *destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	char *dst = (char *)destination;

	while (length--)
		dst[length] = chr;

	return destination;
}

char * itoa(int value){
	static char buffer[20]; // Sufficient for 64-bit integers
	int i = 0;
	int isNegative = 0;

	if (value < 0) {
		isNegative = 1;
		value = -value;
	}

	do {
		buffer[i++] = (value % 10) + '0';
		value /= 10;
	} while (value != 0);

	if (isNegative) {
		buffer[i++] = '-';
	}

	buffer[i] = '\0';

	// Reverse the string
	for (int j = 0; j < i / 2; j++) {
		char temp = buffer[j];
		buffer[j] = buffer[i - j - 1];
		buffer[i - j - 1] = temp;
	}

	return buffer;
}
	

void *memcpy(void *destination, const void *source, uint64_t length)
{
	/*
	 * memcpy does not support overlapping buffers, so always do it
	 * forwards. (Don't change this without adjusting memmove.)
	 *
	 * For speedy copying, optimize the common case where both pointers
	 * and the length are word-aligned, and copy word-at-a-time instead
	 * of byte-at-a-time. Otherwise, copy by bytes.
	 *
	 * The alignment logic below should be portable. We rely on
	 * the compiler to be reasonably intelligent about optimizing
	 * the divides and modulos out. Fortunately, it is.
	 */
	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
		(uint64_t)source % sizeof(uint32_t) == 0 &&
		length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *)destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t *d = (uint8_t *)destination;
		const uint8_t *s = (const uint8_t *)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

// Provide a freestanding implementation of strlen for the kernel.
size_t strlen(const char *str)
{
	size_t len = 0;
	while (str != NULL && str[len] != '\0')
	{
		len++;
	}
	return len;
}


int strcmp(const char *str1, const char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return (str1 == str2) ? 0 : (str1 == NULL ? -1 : 1);
	
	while (*str1 != '\0' && *str2 != '\0')
	{
		if (*str1 != *str2)
			return (*str1 - *str2);
		str1++;
		str2++;
	}
	
	return (*str1 - *str2);
}
