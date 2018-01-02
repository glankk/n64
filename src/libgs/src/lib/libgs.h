#ifndef LIBGS_H
#define LIBGS_H
#include <stdint.h>
#include <libusb-1.0/libusb.h>

#define GS_ERROR_LENGTH 512

enum gs_error
{
  GS_SUCCESS,
  GS_ERROR,
  GS_ERROR_MAX,
};

enum gs_mode
{
  GS_MODE_CAREFUL,
  GS_MODE_STANDARD,
  GS_MODE_FAST,
  GS_MODE_BULK,
};

struct gs_context;
typedef void (*gs_progress_callback_t)(struct gs_context *gs, int chunk_size);
struct gs_context
{
  struct libusb_context       *usb_context;
  struct libusb_device_handle *usb_device;

  enum gs_mode mode;
  int spin_max;
  int timeout;
  int writes_pending;
  gs_progress_callback_t progress_callback;
  void *user_data;

  enum gs_error error_code;
  char error_description[GS_ERROR_LENGTH];
};

struct gs_code
{
  uint32_t address;
  uint16_t value;
};

/* error handling */
void gs_set_error(struct gs_context *gs, enum gs_error error_code,
                  const char *error_format, ...);
void gs_clear_error(struct gs_context *gs);
enum gs_error gs_get_error_code(struct gs_context *gs);
const char *gs_get_error_description(struct gs_context *gs);
void gs_copy_error(struct gs_context *gs, enum gs_error *error_code,
                   char (*error_description)[GS_ERROR_LENGTH]);
int gs_error_succeeded(enum gs_error);
int gs_error_failed(enum gs_error);
int gs_succeeded(struct gs_context *gs);
int gs_failed(struct gs_context *gs);

/* device setup */
struct gs_context *gs_init();
void gs_cleanup(struct gs_context *gs);
void gs_set_mode(struct gs_context *gs, enum gs_mode mode);

/* operations */
void gs_connect(struct gs_context *gs);
void gs_disconnect(struct gs_context *gs);
void gs_handshake(struct gs_context *gs);
char *gs_get_version(struct gs_context *gs);
void gs_ram_read(struct gs_context *gs, uint32_t address,
                 void *data, uint32_t data_size);
void gs_ram_write(struct gs_context *gs, uint32_t address,
                  const void *data, uint32_t data_size,
                  gs_progress_callback_t progress_callback);

/* misc */
void *gs_get_user_data(struct gs_context *gs);
void gs_set_user_data(struct gs_context *gs, void *user_data);

#endif
