#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int strlen(const char * str) {
    int i = 0;
    while (str[i] != 0) {
        i++;
    }
    return i;
}

int strcmp(const char * str1, const char * str2) {
    int i = 0;
    while (str1[i] != 0 && str2[i] != 0) {
        if (str1[i] != str2[i]) {
            return str1[i] - str2[i];
        }
        i++;
    }
    return str1[i] - str2[i];
}

int strcasecmp(char * str1, char * str2) {
    int i = 0;
    while (str1[i] != 0 && str2[i] != 0) {
        if (toupper(str1[i]) != toupper(str2[i])) {
            return str1[i] - str2[i];
        }
        i++;
    }
    return str1[i] - str2[i];
}

void strcpy(char * dest, char * src) {
    int i = 0;
    while (src[i] != 0) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

void strncpy(char * dest, char * src, int n) {
    int i = 0;
    while (src[i] != 0 && i < n) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

// Divide el string s1 en tokens, usando los caracteres en s2 como delimitadores.
//s1 = str: El string a dividir (o NULL para continuar)
//s2 = delim: Los caracteres delimitadores (ej: " ", ",", etc.)
//Return: Puntero al siguiente token, o NULL si no hay más
char * strtok(char * s1, const char * s2) 
{
    static char * last;

    if (s1 != NULL) 
    {
        last = s1;
    } 
    
    else 
    {
        if (*last == 0) 
        {
            return NULL;
        }
        s1 = last;
    }

    while (*last != 0) 
    {
        int i = 0;
        while (s2[i] != 0) 
        {
            if (*last == s2[i]) 
            {
                *last = 0;
                last++;
                return s1;
            }
            i++;
        }
        last++;
    }

    return s1;
}

// Busca la primera ocurrencia del carácter c en el string s
// s: El string donde buscar
// c: El carácter a buscar
// Return: Puntero a la primera ocurrencia de c en s, o NULL si no se encuentra
char * strchr(const char * s, int c) 
{
    while (*s != 0) 
    {
        if (*s == c) 
        {
            return (char *)s;
        }
        s++;
    }
    
    // Check if we're looking for the null terminator
    if (c == 0) 
    {
        return (char *)s;
    }
    
    return NULL;
}

// Copia n bytes de src a dest, manejando correctamente overlapping memory
// dest: Puntero al destino
// src: Puntero al origen
// n: Número de bytes a copiar
// Return: Puntero al destino
void * memmove(void * dest, const void * src, int n) 
{
    char * d = (char *)dest;
    const char * s = (const char *)src;
    
    if (d == s || n == 0) 
    {
        return dest;
    }
    
    if (d < s) 
    {
        // Copy forward
        for (int i = 0; i < n; i++) 
        {
            d[i] = s[i];
        }
    } 
    else 
    {
        // Copy backward to handle overlap
        for (int i = n - 1; i >= 0; i--) 
        {
            d[i] = s[i];
        }
    }
    
    return dest;
}


// Convierte un entero a string en base 10
// num: El número a convertir
// str: Buffer donde escribir el resultado
// Return: Número de caracteres escritos
int int_to_string(int num, char *str)
{
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }
    
    int i = 0;
    int is_negative = 0;
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Contar dígitos y construir string al revés
    int temp = num;
    int digit_count = 0;
    while (temp != 0) {
        digit_count++;
        temp /= 10;
    }
    
    // Agregar signo negativo si es necesario
    if (is_negative) {
        str[i++] = '-';
    }
    
    // Construir los dígitos
    int start = i;
    i += digit_count - 1;
    str[i + 1] = '\0';
    
    while (num != 0) {
        str[i] = (num % 10) + '0';
        num /= 10;
        i--;
    }
    
    return digit_count + (is_negative ? 1 : 0);
}

