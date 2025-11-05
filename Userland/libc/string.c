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
static int int_to_string(int num, char *str)
{
    int i = 0;
    int is_negative = 0;
    
    if (num == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }
    
    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }
    
    // Convertir dígitos en orden inverso
    while (num > 0)
    {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    if (is_negative)
    {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Invertir la cadena
    int len = i;
    for (int j = 0; j < len / 2; j++)
    {
        char temp = str[j];
        str[j] = str[len - 1 - j];
        str[len - 1 - j] = temp;
    }
    
    return len;
}

// Implementación básica de sprintf
// dest: Buffer de destino
// format: String de formato
// ...: Argumentos variables
// Return: Número de caracteres escritos
int sprintf(char *dest, const char *format, ...)
{
    char *d = dest;
    const char *f = format;
    int written = 0;
    
    // Obtener argumentos variables (implementación simple)
    // Asumimos que los argumentos están en el stack después de format
    int *args = (int *)(&format + 1);
    int arg_index = 0;
    
    while (*f != '\0')
    {
        if (*f == '%' && *(f + 1) != '\0')
        {
            f++; // Saltar el '%'
            
            switch (*f)
            {
                case 'd': // Entero decimal
                {
                    char num_str[32];
                    int len = int_to_string(args[arg_index++], num_str);
                    for (int i = 0; i < len; i++)
                    {
                        *d++ = num_str[i];
                        written++;
                    }
                    break;
                }
                case 's': // String
                {
                    char *str = (char *)args[arg_index++];
                    if (str != NULL)
                    {
                        while (*str != '\0')
                        {
                            *d++ = *str++;
                            written++;
                        }
                    }
                    break;
                }
                case 'c': // Carácter
                {
                    *d++ = (char)args[arg_index++];
                    written++;
                    break;
                }
                case '%': // Literal '%'
                {
                    *d++ = '%';
                    written++;
                    break;
                }
                default:
                    // Especificador no soportado, copiar literal
                    *d++ = '%';
                    *d++ = *f;
                    written += 2;
                    break;
            }
        }
        else
        {
            // Carácter normal, copiar directamente
            *d++ = *f;
            written++;
        }
        f++;
    }
    
    *d = '\0'; // Null terminator
    return written;
}
