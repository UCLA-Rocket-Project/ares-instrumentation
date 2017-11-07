#ifndef TEXT_PLOT_H
#define TEXT_PLOT_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct {
  double a;
  double b;
} point2D;

typedef struct {
  char* canvas;
  double* x;
  double* y;
  int N;
  double minx;
  double maxx;
  double miny;
  double maxy;
  char free;
  char clear;
  char draw;
  int n;
  int m;
  char marker;
} text_plot_config;

void text_plot(text_plot_config* config);
int ascPoint2D(const void* a, const void* b);





#endif /* TEXT_PLOT_H */
