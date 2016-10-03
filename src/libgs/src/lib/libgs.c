#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>
#include <libusb.h>
#include "libgs.h"
#include "mos.h"
#include "mos_io.h"
#include "gs_io.h"

#define GS_MAX_ATTEMPTS 16

static void gs_ram_write_start(struct gs_context *gs, uint32_t address,
                               size_t data_size);
static void gs_ram_write_byte(struct gs_context *gs, uint8_t data);
static void gs_ram_write_end(struct gs_context *gs, uint8_t checksum);
static uint8_t gs_end_transaction(struct gs_context *gs, uint8_t checksum);
static void gs_handle_events(struct gs_context *gs, int timeout,
                             int max_pending_writes);

static const struct timespec ts_100ms = {0, 100 * 1000 * 1000};
static const char *gs_error_name[GS_ERROR_MAX] =
{
  "success",
  "error",
};

static enum gs_error gs_init_error_code;
static char gs_init_error_description[GS_ERROR_LENGTH];


void gs_set_error(struct gs_context *gs, enum gs_error error_code,
                  const char *error_format, ...)
{
  gs->error_code = error_code;
  if (error_format) {
    va_list va_args;
    va_start(va_args, error_format);
    vsnprintf(gs->error_description, sizeof(gs->error_description),
              error_format, va_args);
    va_end(va_args);
  }
  else
    gs->error_description[0] = 0;
}

void gs_clear_error(struct gs_context *gs)
{
  gs->error_code = GS_SUCCESS;
  gs->error_description[0] = 0;
}

enum gs_error gs_get_error_code(struct gs_context *gs)
{
  return (gs ? gs->error_code : gs_init_error_code);
}

const char *gs_get_error_description(struct gs_context *gs)
{
  enum gs_error error_code = gs_get_error_code(gs);
  const char *error_description = (char*)(gs ? &gs->error_description :
                                          &gs_init_error_description);
  if (error_description[0])
    return error_description;
  else if (error_code >= 0 && error_code < GS_ERROR_MAX)
    return gs_error_name[error_code];
  else
    return "unknown error";
}

void gs_copy_error(struct gs_context *gs, enum gs_error *error_code,
                   char (*error_description)[GS_ERROR_LENGTH])
{
  if (error_code)
    *error_code = gs_get_error_code(gs);
  if (error_description)
    strcpy((char*)error_description, gs_get_error_description(gs));
}

int gs_error_succeeded(enum gs_error error_code)
{
  return error_code == GS_SUCCESS;
}

int gs_error_failed(enum gs_error error_code)
{
  return error_code != GS_SUCCESS;
}

int gs_succeeded(struct gs_context *gs)
{
  return gs_error_succeeded(gs_get_error_code(gs));
}

int gs_failed(struct gs_context *gs)
{
  return gs_error_failed(gs_get_error_code(gs));
}

struct gs_context *gs_init()
{
  enum libusb_error usb_error;
  struct gs_context *gs = calloc(1, sizeof(struct gs_context));

  usb_error = libusb_init(&gs->usb_context);
  if (usb_error != LIBUSB_SUCCESS) {
    gs_set_error(gs, GS_ERROR, "libusb_init failed: %s",
                 libusb_error_name(usb_error));
    goto clean;
  }

  gs->usb_device = libusb_open_device_with_vid_pid(gs->usb_context,
                                                   VENDOR_ID, PRODUCT_ID);
  if (!gs->usb_device) {
    gs_set_error(gs, GS_ERROR, "libusb_open_device_with_vid_pid failed");
    goto clean;
  }

  usb_error = libusb_claim_interface(gs->usb_device, 0);
  if (usb_error != LIBUSB_SUCCESS) {
    gs_set_error(gs, GS_ERROR, "libusb_claim_interface failed: %s",
                 libusb_error_name(usb_error));
    goto clean;
  }

  gs_set_mode(gs, GS_MODE_STANDARD);
  if (gs_failed(gs))
    goto clean;
  gs_mos_set_mode(gs, MOS_SPP_MODE);
  if (gs_failed(gs))
    goto clean;
  gs->spin_max = 10000;
  gs->timeout = 10 * 1000;
  gs->writes_pending = 0;
  gs->progress_callback = NULL;

  gs_clear_error(gs);
  goto exit;

clean:
  gs_copy_error(gs, &gs_init_error_code, &gs_init_error_description);
  gs_cleanup(gs);
  gs = NULL;

exit:
  return gs;
}

void gs_cleanup(struct gs_context *gs)
{
  if (!gs)
    return;
  if (gs->usb_device)
    libusb_release_interface(gs->usb_device, 0);
  if (gs->usb_context)
    libusb_exit(gs->usb_context);
  free(gs);
}

void gs_set_mode(struct gs_context *gs, enum gs_mode mode)
{
  switch (mode) {
    case GS_MODE_CAREFUL:
    case GS_MODE_STANDARD:
    case GS_MODE_FAST:
    case GS_MODE_BULK:
      gs->mode = mode;
      gs_clear_error(gs);
      break;
    default:
      gs_set_error(gs, GS_ERROR, "gs_set_mode failed: mode #%d unsupported",
                   mode);
      break;
  }
}

void gs_connect(struct gs_context *gs)
{
  uint8_t data_in;
  for (int i = 0; i < GS_MAX_ATTEMPTS; ++i) {
    gs_mos_write(gs, 3, 1);
    if (gs_failed(gs))
      return;

    nanosleep(&ts_100ms, NULL);
    data_in = gs_mos_read(gs) & 0x0F;
    if (gs_failed(gs))
      return;
    gs_mos_clear(gs);
    if (gs_failed(gs))
      return;
    nanosleep(&ts_100ms, NULL);

    gs_mos_write(gs, 3, 1);
    if (gs_failed(gs))
      return;
    nanosleep(&ts_100ms, NULL);
    data_in = (data_in << 4) | (gs_mos_read(gs) & 0x0F);
    if (gs_failed(gs))
      return;
    gs_mos_clear(gs);
    if (gs_failed(gs))
      return;
    nanosleep(&ts_100ms, NULL);

    if (data_in == 'g') {
      gs_clear_error(gs);
      return;
    }

    /* try to get back in sync */
    gs_mos_write(gs, 3, 1);
    if (gs_failed(gs))
      return;
    nanosleep(&ts_100ms, NULL);
    gs_mos_clear(gs);
    if (gs_failed(gs))
      return;
    nanosleep(&ts_100ms, NULL);
  }

  gs_mos_clear(gs);
}

void gs_disconnect(struct gs_context *gs)
{
  for (int i = 0; i < GS_MAX_ATTEMPTS; ++i) {
    gs_handshake(gs);
    if (gs_succeeded(gs)) {
      gs_readwrite_u8(gs, 'd');
      return;
    }
  }
  gs_set_error(gs, GS_ERROR, "gs_disconnect failed");
}

void gs_handshake(struct gs_context *gs)
{
  char data_in[2];
  gs_readwrite(gs, data_in, "GT", 2);
  if (gs_failed(gs))
    return;
  if (memcmp(&data_in, "gt", 2) != 0) {
    gs_set_error(gs, GS_ERROR, "gs_handshake failed (0x%02x, 0x%02x)",
                 data_in[0], data_in[1]);
    return;
  }
  gs_clear_error(gs);
}

char *gs_get_version(struct gs_context *gs)
{
  gs_handshake(gs);
  if (gs_failed(gs))
    return NULL;

  gs_readwrite_u8(gs, 'f');
  if (gs_failed(gs))
    return NULL;
  gs_read(gs, NULL, 3);
  if (gs_failed(gs))
    return NULL;

  int version_length = gs_read_u8(gs);
  if (gs_failed(gs))
    return NULL;
  char *version_string = malloc(version_length + 1);
  gs_read(gs, version_string, version_length);
  if (gs_failed(gs)) {
    free(version_string);
    return NULL;
  }
  version_string[version_length] = 0;

  gs_clear_error(gs);
  return version_string;
}

void gs_ram_read(struct gs_context *gs, uint32_t address,
                 void *data, uint32_t data_size)
{
  uint8_t *c_data = data;
  gs_handshake(gs);
  if (gs_failed(gs))
    return;

  gs_readwrite_u8(gs, 1);
  if (gs_failed(gs))
    return;
  gs_readwrite_u32(gs, address);
  if (gs_failed(gs))
    return;
  gs_readwrite_u32(gs, data_size);
  if (gs_failed(gs))
    return;

  for (uint32_t i = 0; i < data_size; ++i) {
    uint8_t byte = gs_read_u8(gs);
    if (gs_failed(gs))
      return;
    if (c_data)
      c_data[i] = byte;
  }

  gs_end_transaction(gs, 0);
}

void gs_ram_write(struct gs_context *gs, uint32_t address,
                  const void *data, uint32_t data_size,
                  gs_progress_callback_t progress_callback)
{
  const uint8_t *c_data = data;
  gs->progress_callback = progress_callback;
  gs_ram_write_start(gs, address, data_size);
  if (gs_failed(gs))
    return;

  uint8_t checksum = 0;
  if (gs->mode == GS_MODE_BULK) {
    const int max_chunk_size = 256;
    int chunk_size;
    for (uint32_t i = 0; i < data_size; i += chunk_size) {
      chunk_size = max_chunk_size;
      if (i + chunk_size > data_size)
        chunk_size = data_size - i;

      for (int j = 0; j < chunk_size; ++j)
        checksum = (checksum + c_data[i + j]) & 0xFF;

      gs_mos_write_bulk_async(gs, c_data + i, chunk_size);
      if (gs_failed(gs))
        return;

      gs_handle_events(gs, 0, 512);
    }
  }
  else {
    for (uint32_t i = 0; i < data_size; ++i) {
      checksum = (checksum + c_data[i]) & 0xFF;

      gs_ram_write_byte(gs, c_data[i]);
      if (gs_failed(gs))
        return;
      if (gs->progress_callback)
        gs->progress_callback(gs, 1);

      gs_handle_events(gs, 0, 256);
    }
  }

  gs_ram_write_end(gs, checksum);
  if (gs_failed(gs))
    return;
}

void gs_ram_write_start(struct gs_context *gs, uint32_t address,
                        uint32_t data_size)
{
  gs_handshake(gs);
  if (gs_failed(gs))
    return;

  gs_readwrite_u8(gs, 2);
  if (gs_failed(gs))
    return;
  gs_readwrite_u32(gs, address);
  if (gs_failed(gs))
    return;

  if (gs->mode == GS_MODE_BULK) {
    /* bulk mode may throw up flags before we detect clear */
    gs_readwrite_u8(gs, data_size >> 24);
    if (gs_failed(gs))
      return;
    gs_readwrite_u8(gs, data_size >> 16);
    if (gs_failed(gs))
      return;
    gs_readwrite_u8(gs, data_size >> 8);
    if (gs_failed(gs))
      return;
    gs_readwrite_u4(gs, data_size >> 4);
    if (gs_failed(gs))
      return;
    gs_write_u4(gs, data_size);
    if (gs_failed(gs))
      return;
  }
  else {
    gs_readwrite_u32(gs, data_size);
    if (gs_failed(gs))
      return;
  }

  if (gs->writes_pending != 0) {
    gs_set_error(gs, GS_ERROR,
                 "gs_ram_write_start failed: "
                 "%i writes pending when starting a new write",
                 gs->writes_pending);
    return;
  }

  if (gs->mode == GS_MODE_BULK) {
    gs_mos_set_mode(gs, MOS_FIFO_MODE);
    if (gs_failed(gs))
      return;
  }

  gs_clear_error(gs);
}

void gs_ram_write_byte(struct gs_context *gs, uint8_t data)
{
  switch (gs->mode) {
    case GS_MODE_CAREFUL:
      gs_readwrite_u8(gs, data);
      break;
    case GS_MODE_STANDARD:
      gs_write_u8(gs, data);
      break;
    case GS_MODE_FAST:
      gs_write_u8_fast(gs, data);
      break;
    case GS_MODE_BULK:
      gs_mos_write_bulk_async(gs, &data, 1);
      break;
  }
}

void gs_ram_write_end(struct gs_context *gs, uint8_t checksum)
{
  if (gs->mode == GS_MODE_BULK) {
    gs_mos_clear_bulk_async(gs);
    if (gs_failed(gs))
      return;
  }

  while (gs->writes_pending > 0) {
    nanosleep(&ts_100ms, NULL);
    gs_handle_events(gs, gs->timeout, 0);
  }

  if (gs->mode == GS_MODE_BULK) {
    gs_mos_set_mode(gs, MOS_SPP_MODE);
    if (gs_failed(gs))
      return;
  }

  uint8_t remote_checksum = gs_end_transaction(gs, 0);
  if (gs_failed(gs))
      return;
  if (checksum != remote_checksum) {
    gs_set_error(gs, GS_ERROR,
                 "gs_ram_write_end failed: checksum mismatch (0x%02x != 0x%02x)",
                 checksum, remote_checksum);
    return;
  }

  gs_clear_error(gs);
}

uint8_t gs_end_transaction(struct gs_context *gs, uint8_t checksum)
{
  gs_readwrite_u32(gs, 0);
  if (gs_failed(gs))
    return 0;
  gs_readwrite_u32(gs, 0);
  if (gs_failed(gs))
    return 0;
  uint8_t result = gs_readwrite_u8(gs, checksum);
  if (gs_failed(gs))
    return 0;
  return result;
}

void gs_handle_events(struct gs_context *gs, int timeout,
                      int max_pending_writes)
{
  if (gs->writes_pending > max_pending_writes) {
    struct timeval tv = {0, timeout * 1000};
    libusb_handle_events_timeout(gs->usb_context, &tv);
  }
}

void *gs_get_user_data(struct gs_context *gs)
{
  return gs->user_data;
}

void gs_set_user_data(struct gs_context *gs, void *user_data)
{
  gs->user_data = user_data;
}
