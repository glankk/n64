#include <stdio.h>
#include <time.h>
#include <math.h>
#include "progress.h"

static void progress_bar_print(struct progress_bar *pb);


void progress_bar_start(struct progress_bar *pb, size_t data_size,
                        clock_t progress_interval)
{
  pb->data_size = data_size;
  pb->data_completed = 0;
  pb->start_time = clock();
  pb->progress_interval = progress_interval;
  pb->progress_time = pb->start_time + pb->progress_interval;
  printf("  0.00%% [                                                  ]"
         "    0.00B/s");
}

void progress_bar_proceed(struct progress_bar *pb, size_t chunk_size)
{
  pb->data_completed += chunk_size;
}

void progress_bar_update(struct progress_bar *pb)
{
  clock_t current_time = clock();
  if (current_time >= pb->progress_time) {
    pb->progress_time += pb->progress_interval;
    progress_bar_print(pb);
  }
}

void progress_bar_end(struct progress_bar *pb)
{
  progress_bar_print(pb);
  printf("\n");
}

void progress_bar_print(struct progress_bar *pb)
{
  printf("\r%6.2f%% [", (float)pb->data_completed / pb->data_size * 100.f);
  for (int j = 0; j < 50; ++j)
    fputc(j < pb->data_completed * 50 / pb->data_size ? '=' : ' ', stdout);
  float rate = (float)pb->data_completed /
               (clock() - pb->start_time) * CLOCKS_PER_SEC;
  printf("] %7.2fB/s", isnormal(rate) ? rate : 0.f);
}
