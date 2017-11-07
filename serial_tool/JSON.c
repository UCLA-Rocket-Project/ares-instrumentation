#include "JSON.h"

int parseJSON(char*** keys, char*** values, const char* JSON_org) {
  if (keys == NULL || values == NULL)
    return 0;
  *keys = NULL;
  *values = NULL;

  uint32_t k = 0;

  uint32_t i = 0;
  uint32_t len = strlen(JSON_org);
  if (len == 0) {
    return 0;
  }
  char* JSON = (char*)malloc((len + 1)* sizeof(char));
  strcpy(JSON, JSON_org);
  strip(JSON, '\n');
  len = strlen(JSON);

  if (JSON[0] != '{' || JSON[len-1] != '}') {
    //printf("JSON not formatted properly\n");
    free(JSON);
    return 0;
  }

  uint32_t pair_nb = 0;
  char word[256];
  uint32_t lock = 0;
  i = 1;
  while (i < len) {
    if (JSON[i] == '{') {
      while (i < len-1 && JSON[i] != '}')
        i++;
    }
    if (JSON[i] == ':' && lock == 0) {
      pair_nb++;
      lock = 1;
    }
    if (JSON[i] == ',' && lock == 1) {
      lock = 0;
    }
    i++;
  }

  *keys = (char**)malloc(pair_nb * sizeof(char*));
  *values = (char**)malloc(pair_nb * sizeof(char*));

  for (k = 0; k < pair_nb; k++) {
    (*keys)[k] = (char*)malloc(256 * sizeof(char));
    (*values)[k] = (char*)malloc(256 * sizeof(char));

    //printf("%x\n%x\n", (*keys)[k], (*values)[k]);
  }
  //printf("pair_nb = %lu\n", pair_nb);

  i = 1;
  uint32_t j = 0;
  k = 0;
  while (i < len && k < pair_nb) {
    j = i;
    while (j < len-1 && JSON[j] != ':') {
      if (JSON[j] == '{') {
        while (j < len-1 && JSON[j] != '}')
          j++;
      }
      j++;
    }
    if ((j < len-1) && (j-i < 256)) {
      memcpy(word, JSON+i, j-i);
      word[j-i] = 0;
      strip(word, ' ');
    } else {
      word[0] = 0;
    }
    //printf("About to strncpy\n");
    strip(word, '"');
    strncpy((*keys)[k], word, 256);
    (*keys)[k][256-1] = 0;
    //printf("Extracted key word: \"%s\"\n", word);

    i = j + 1;
    
    j++;
    while (j < len-1 && JSON[j] != ',') {
      if (JSON[j] == '{') {
        while (j < len-1 && JSON[j] != '}')
          j++;
      }
      j++;
    }
    if ((j <= len-1) && (j-i < 256)) {
      memcpy(word, JSON+i, j-i);
      word[j-i] = 0;
      strip(word, ' ');
    } else {
      word[0] = 0;
    }
    strip(word, '"');
    strncpy((*values)[k], word, 256);
    (*values)[k][256-1] = 0;
    //printf("Extracted value word: \"%s\"\n", word);
    
    i = j + 1;
    k++;
  }

  free(JSON);
  return pair_nb;
}

void strip(char* s, char c) {
  //printf("Stripping \"%s\"\n", s);
  int32_t i = 0;
  uint32_t b, e;
  uint32_t len = strlen(s);
  while (i < len && s[i] == c)
    i++;
  b = i;
  for (i = len-1; i >= 0 && s[i] == c; i--) {
    asm("nop");
  }

  e = i + 1;
  char temp[1024];
  if (e - b + 1 >= 1024)
    return;

  //printf("b = %lu, e = %lu\n", b, e);
  memcpy(temp, s + b * sizeof(char), (e - b) * sizeof(char));
  temp[e - b] = 0;
  //printf("temp = \"%s\"\n", temp);
  strncpy(s, temp, len);
  s[len] = 0;
}

void free_JSON_array(char** arr, uint32_t n) {
  if (arr == NULL)
    return;
  uint32_t i;
  for (i = 0; i < n; i++)
    free(arr[i]);
  free(arr);
  arr = NULL;
}
