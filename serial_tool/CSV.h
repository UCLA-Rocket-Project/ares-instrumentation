#ifndef CSV_H
#define CSV_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int parse_CSV(char* csv, char*** fields);
void free_CSV_fields(char** fields, int n);
#endif
