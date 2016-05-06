#ifndef PROGRESS_H
#define PROGRESS_H
#include <time.h>

struct progress_bar
{
  size_t data_size;
  size_t data_completed;
  clock_t start_time;
  clock_t progress_interval;
  clock_t progress_time;
};

void progress_bar_start(struct progress_bar *pb, size_t data_size,
                        clock_t progress_interval);
void progress_bar_proceed(struct progress_bar *pb, size_t chunk_size);
void progress_bar_update(struct progress_bar *pb);
void progress_bar_end(struct progress_bar *pb);

#endif
