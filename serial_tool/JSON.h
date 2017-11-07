#ifndef JSON_H
#define JSON_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int parseJSON(char*** keys, char*** values, const char* JSON_org);
void strip(char* s, char c);
void free_JSON_array(char** arr, uint32_t n);


#endif
