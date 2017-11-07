#include "text_plot.h"

void text_plot(text_plot_config* config) {
  int offsetn = 10;
  int offsetm = 1;
  if (config->canvas == NULL)
    config->canvas = malloc(sizeof(char)*(config->n + offsetn)*(config->m + offsetm));
  point2D* points = malloc(sizeof(point2D)*(config->N));
  int i, j;
  double minx, maxx, rngx, miny, maxy, rngy;
  /*
  miny = y[0];
  maxy = y[0];
  for (i = 0; i < N; i++) {
    double xval = x[i];
    double yval = y[i];
    points[i].a = xval;
    points[i].b = yval;
    maxx = xval > maxx ? xval : maxx;
    minx = xval < minx ? xval : minx;
    maxy = yval > maxy ? yval : maxy;
    miny = yval < miny ? yval : miny;
  }
  rngx = maxx - minx;
  rngy = maxy - miny;
  */
  minx = config->minx;
  maxx = config->maxx;
  miny = config->miny;
  maxy = config->maxy;
  rngx = maxx - minx;
  rngy = maxy - miny;
  double* x = config->x;
  double* y = config->y;

  for (i = 0; i < config->N; i++) {
    double xval = x[i];
    double yval = y[i];
    points[i].a = xval;
    points[i].b = yval;
  }

  qsort(points, config->N, sizeof(point2D), ascPoint2D);

  //printf("minx -> %f, maxx -> %f, rngx -> %f\n", minx, maxx, rngx);
  //printf("miny -> %f, maxy -> %f, rngy -> %f\n", miny, maxy, rngy);

  int N = config->N;
  int n = config->n;
  int m = config->m;
  int width = n + offsetn;
  int height = m + offsetm;

  if (config->clear) {
    memset(config->canvas, ' ', width*height*sizeof(char));
    sprintf(config->canvas, "%+3.2e", maxy);
    sprintf(config->canvas + ((m-1) * width), "%+3.2e", miny);
    sprintf(config->canvas + (width*(m + offsetm - 1) + offsetn - 4), "%+3.2e", minx);
    sprintf(config->canvas + (width*(m + offsetm - 1) + n + offsetn - 9), "%+3.2e", maxx);
    for (i = offsetn; i < width; i++)
      config->canvas[(width)*(m - 1) + i] = '-';
    for (i = 0; i < m; i++)
      config->canvas[(width)*(i) + offsetn] = '|';
    config->canvas[offsetn] = '^';
    config->canvas[(width)*(m - 1) + width - 1] = '>';
    config->canvas[(width)*(m - 1) + offsetn] = '+';
    for (i = 0; i < width*height; i++)
      if (config->canvas[i] == 0)
        config->canvas[i] = ' ';
        
  }
  int step = (int)floor((double)N / (double)n);
  int half_step = step / 2;
  for (i = half_step; i < N; i += step) {
    int count = 1;
    double xval = points[i].a;
    double yval = points[i].b;
    for (j = i - half_step; j < i + half_step; j++)
      if (j != i) {
        xval += points[j].a;
        yval += points[j].b;
        count++;
      }
    xval /= (double)count;
    yval /= (double)count;
    int xpos = (int)round((double)(n-1) * (xval - minx) / rngx);
    int ypos = (int)round((double)(m-1) * (yval - miny) / rngy);
    if (xpos < (n + offsetn) && xpos >= 0 && ypos < m && ypos >= 0) {
      config->canvas[(width)*(m - ypos - 1) + (offsetn + xpos)] = config->marker;
    }
  }


  if (config->draw) {
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
        printf("%c", config->canvas[(width)*j + i]);
      }
      printf("\n");
    }
  }
  if (config->free)
    free(config->canvas);

  free(points);
}

int ascPoint2D(const void* a, const void* b) {
  return ((point2D*)a)->a > ((point2D*)b)->a ? 1 : -1;
}
