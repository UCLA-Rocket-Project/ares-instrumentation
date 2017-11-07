#include "CSV.h"

int parse_CSV(char* csv, char*** fields) {

  int csv_len = strlen(csv);
  int n = 1;
  int i;
  for (i = 0; i < csv_len; i++)
    if (csv[i] == ',') n++;
  *fields = (char**)malloc(sizeof(char*)*n);
  int j = 0;
  int k = 0;
  for (i = 0; i < csv_len; i++) {
    if (csv[i] == ',') {
      (*fields)[k] = (char*)malloc(sizeof(char)*(i-j+1));
      memcpy((*fields)[k], csv+j, i-j);
      (*fields)[k][i-j] = 0;
      j = i + 1;
      k++;
    }
  }
  (*fields)[k] = (char*)malloc(sizeof(char)*(csv_len-j+1));
  memcpy((*fields)[k], csv+j, csv_len-j);
  (*fields)[k][csv_len-j] = 0;

  //for (i = 0; i < n; i++) printf("%s\n", (*fields)[i]); 
  return n;
}

void free_CSV_fields(char** fields, int n) {
  int i;
  if (fields == NULL) return;
  for (i = 0; i < n; i++) {
    if (fields[i] != NULL) free(fields[i]);
  }
  free(fields);
}
