#include <sys/time.h>
#include <mips.h>
#include "../lib/libgs.h"
#include "../lib/mos_io.h"
#include "progress.h"
#include "ops.h"

static void write_progress_callback(struct gs_context *gs, int chunk_size);


void write_codes(struct gs_code *codes, size_t num_codes,
                 size_t data_size, enum gs_error *gs_error_code,
                 char (*gs_error_description)[GS_ERROR_LENGTH])
{
  struct gs_context *gs = gs_init();
  if (gs_failed(gs))
    goto exit;
  gs_connect(gs);
  if (gs_failed(gs))
    goto exit;
  struct progress_bar pb;
  progress_bar_start(&pb, data_size, CLOCKS_PER_SEC / 4);
  for (size_t i = 0; i < num_codes; ++i) {
    progress_bar_update(&pb);
    if ((codes[i].address & 0xFF000000) == 0x80000000) {
      uint8_t value = codes[i].value;
      gs_ram_write(gs, codes[i].address, &value, 1, NULL);
      progress_bar_proceed(&pb, 1);
    }
    else if ((codes[i].address & 0xFF000000) == 0x81000000) {
      uint16_t value = mips_htom16(codes[i].value);
      gs_ram_write(gs, codes[i].address - 0x01000000, &value, 2, NULL);
      progress_bar_proceed(&pb, 2);
    }
    else
      continue;
    if (gs_failed(gs))
      break;
  }
  progress_bar_end(&pb);
  if (gs_failed(gs))
    goto exit;
  gs_disconnect(gs);
exit:
  gs_copy_error(gs, gs_error_code, gs_error_description);
  gs_cleanup(gs);
}

void write_raw(uint32_t address, void *data, size_t data_size,
               size_t chunk_size, enum gs_error *gs_error_code,
               char (*gs_error_description)[GS_ERROR_LENGTH])
{
  if (chunk_size == 0)
    chunk_size = data_size;
  struct gs_context *gs = gs_init();
  if (gs_failed(gs))
    goto exit;
  struct progress_bar pb;
  progress_bar_start(&pb, data_size, CLOCKS_PER_SEC / 4);
  gs_set_user_data(gs, &pb);
  for (size_t i = 0; i < data_size; i += chunk_size) {
    if (i > 0) {
      struct timespec ts_100ms = {0, 100 * 1000 * 1000};
      nanosleep(&ts_100ms, NULL);
    }
    if (i + chunk_size > data_size)
      chunk_size = data_size - i;
    gs_connect(gs);
    if (gs_failed(gs))
      break;
    gs_ram_write(gs, address + i, (char*)data + i, chunk_size,
                 write_progress_callback);
    if (gs_failed(gs))
      break;
    gs_disconnect(gs);
    if (gs_failed(gs))
      break;
  }
  progress_bar_end(&pb);
exit:
  gs_copy_error(gs, gs_error_code, gs_error_description);
  gs_cleanup(gs);
}

void write_progress_callback(struct gs_context *gs, int chunk_size)
{
  struct progress_bar *pb = gs_get_user_data(gs);
  progress_bar_proceed(pb, chunk_size);
  progress_bar_update(pb);
}
