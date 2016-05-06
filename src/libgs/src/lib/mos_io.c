#include <stdint.h>
#include <unistd.h>
#include "libgs.h"
#include "mos.h"

static void gs_mos_write_async_proc(struct libusb_transfer *usb_transfer);
static void gs_mos_write_bulk_async_proc(struct libusb_transfer *usb_transfer);
static void gs_mos_clear_bulk_async_proc(struct libusb_transfer *usb_transfer);


void gs_mos_set_mode(struct gs_context *gs, int mos_mode)
{
  enum libusb_error usb_error =
      libusb_control_transfer(gs->usb_device,
                              REQTYPE_WRITE,
                              REQ_MOS_WRITE,
                              MOS_PORT_BASE | mos_mode,
                              MOS_PP_EXTENDED_CONTROL_REG,
                              NULL,
                              0,
                              gs->timeout
                             );

  if (usb_error != LIBUSB_SUCCESS) {
    gs_set_error(gs, GS_ERROR, "libusb_control_transfer failed: %s",
                 libusb_error_name(usb_error));
    return;
  }

  gs_clear_error(gs);
}

uint8_t gs_mos_read_raw(struct gs_context *gs)
{
  uint8_t data_in;
  int usb_transfer_size =
      libusb_control_transfer(
                              gs->usb_device,
                              REQTYPE_READ,
                              REQ_MOS_READ,
                              MOS_PORT_BASE,
                              MOS_PP_STATUS_REG,
                              &data_in,
                              1,
                              gs->timeout
                             );

  if (usb_transfer_size != 1) {
    gs_set_error(gs, GS_ERROR, "libusb_control_transfer failed: %s",
                 libusb_error_name(usb_transfer_size));
    return 0;
  }

  gs_clear_error(gs);
  return data_in;
}

uint8_t gs_mos_read(struct gs_context *gs)
{
  uint8_t data = gs_mos_read_raw(gs);
  if (gs_failed(gs))
    return 0;
  gs_clear_error(gs);
  if (data & 0x08)
    return ((data ^ 0x80) >> 4) | 0x10;
  return 0;
}

void gs_mos_write(struct gs_context *gs, uint8_t data, int flagged)
{
  data = (flagged ? 0x10 : 0x00) | (data & 0x0F);

  int usb_transfer_size =
      libusb_control_transfer(
                              gs->usb_device,
                              REQTYPE_WRITE,
                              REQ_MOS_WRITE,
                              MOS_PORT_BASE | data,
                              MOS_PP_DATA_REG,
                              NULL,
                              0,
                              gs->timeout
                             );

  if (usb_transfer_size != 0) {
    gs_set_error(gs, GS_ERROR, "libusb_control_transfer: %s",
                 libusb_error_name(usb_transfer_size));
    return;
  }

  gs_clear_error(gs);
}

void gs_mos_write_async_proc(struct libusb_transfer *usb_transfer)
{
  struct gs_context *gs = usb_transfer->user_data;
  --gs->writes_pending;
  if (gs->progress_callback)
    gs->progress_callback(gs, usb_transfer->actual_length);
}

void gs_mos_write_async(struct gs_context *gs, uint8_t data, int flagged)
{
  data = (flagged ? 0x10 : 0x00) | (data & 0x0F);

  struct libusb_transfer *usb_transfer = libusb_alloc_transfer(0);
  uint8_t setup_buffer[LIBUSB_CONTROL_SETUP_SIZE];

  libusb_fill_control_setup(
                            setup_buffer,
                            REQTYPE_WRITE,
                            REQ_MOS_WRITE,
                            MOS_PORT_BASE | data,
                            MOS_PP_DATA_REG,
                            0
                           );

  libusb_fill_control_transfer(
                               usb_transfer,
                               gs->usb_device,
                               setup_buffer,
                               (libusb_transfer_cb_fn)
                               gs_mos_write_async_proc,
                               gs,
                               gs->timeout
                              );

  usb_transfer->flags |= LIBUSB_TRANSFER_FREE_TRANSFER;
  usb_transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  usb_transfer->flags |= LIBUSB_TRANSFER_SHORT_NOT_OK;

  enum libusb_error usb_error = libusb_submit_transfer(usb_transfer);
  if (usb_error != LIBUSB_SUCCESS) {
    gs_set_error(gs, GS_ERROR, "libusb_submit_transfer failed: %s",
                 libusb_error_name(usb_error));
    return;
  }

  ++gs->writes_pending;
  gs_clear_error(gs);
}

void gs_mos_clear(struct gs_context *gs)
{
  gs_mos_write(gs, 0, 0);
}

void gs_mos_clear_async(struct gs_context *gs)
{
  gs_mos_write_async(gs, 0, 0);
}

void gs_mos_write_fast(struct gs_context *gs, uint8_t data)
{
  gs_mos_write(gs, (data >> 4) & 0x0F, 1);
  if (gs_failed(gs))
    return;
  gs_mos_write(gs, (data >> 0) & 0x0F, 0);
}

void gs_mos_write_fast_async(struct gs_context *gs, uint8_t data)
{
  gs_mos_write_async(gs, (data >> 4) & 0x0F, 1);
  if (gs_failed(gs))
    return;
  gs_mos_write_async(gs, (data >> 0) & 0x0F, 0);
}

void gs_mos_write_bulk_async_proc(struct libusb_transfer *usb_transfer)
{
  struct gs_context *gs = usb_transfer->user_data;
  --gs->writes_pending;
  if (gs->progress_callback)
    gs->progress_callback(gs, usb_transfer->actual_length);
}

void gs_mos_write_bulk_async(struct gs_context *gs, const void *data,
                             size_t data_size)
{
  const int max_transfers = 32;
  const int transfers_per_byte = 2;
  const int chunk_size = max_transfers / transfers_per_byte;
  const uint8_t *c_data = data;

  while (data_size > 0) {
    struct libusb_transfer *usb_transfer = libusb_alloc_transfer(0);

    int transfer_data_size = chunk_size;
    if (transfer_data_size > data_size)
      transfer_data_size = data_size;

    uint8_t chunk_data[max_transfers];

    for (int i = 0, j = 0; i < transfer_data_size;
         ++i, j += transfers_per_byte)
    {
      chunk_data[j + 0] = 0x10 | ((c_data[i] >> 4) & 0x0F);
      chunk_data[j + 1] = 0x00 | ((c_data[i] >> 0) & 0x0F);
    }

    libusb_fill_bulk_transfer(
                              usb_transfer,
                              gs->usb_device,
                              ENDPOINT_MOS_BULK_WRITE,
                              chunk_data,
                              transfer_data_size * transfers_per_byte,
                              (libusb_transfer_cb_fn)
                              gs_mos_write_bulk_async_proc,
                              gs,
                              gs->timeout
                             );

    usb_transfer->flags |= LIBUSB_TRANSFER_FREE_TRANSFER;
    usb_transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
    usb_transfer->flags |= LIBUSB_TRANSFER_SHORT_NOT_OK;

    enum libusb_error usb_error = libusb_submit_transfer(usb_transfer);
    if (usb_error != LIBUSB_SUCCESS) {
      gs_mos_clear(gs);
      gs_mos_set_mode(gs, MOS_SPP_MODE);
      gs_set_error(gs, GS_ERROR, "libusb_submit_transfer failed: %s",
                   libusb_error_name(usb_error));
      return;
    }

    ++gs->writes_pending;
    data_size -= transfer_data_size;
    c_data += transfer_data_size;
  }

  gs_clear_error(gs);
}

void gs_mos_clear_bulk_async_proc(struct libusb_transfer *usb_transfer)
{
  struct gs_context *gs = usb_transfer->user_data;
  --gs->writes_pending;
}

void gs_mos_clear_bulk_async(struct gs_context *gs)
{
  struct libusb_transfer *usb_transfer = libusb_alloc_transfer(0);

  uint8_t data = 0;
  libusb_fill_bulk_transfer(usb_transfer,
                            gs->usb_device,
                            ENDPOINT_MOS_BULK_WRITE,
                            &data,
                            1,
                            (libusb_transfer_cb_fn)
                            gs_mos_clear_bulk_async_proc,
                            gs,
                            gs->timeout);

  usb_transfer->flags |= LIBUSB_TRANSFER_FREE_TRANSFER;
  usb_transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  usb_transfer->flags |= LIBUSB_TRANSFER_SHORT_NOT_OK;

  enum libusb_error usb_error = libusb_submit_transfer(usb_transfer);
  if (usb_error != LIBUSB_SUCCESS) {
    gs_mos_clear(gs);
    gs_mos_set_mode(gs, MOS_SPP_MODE);
    gs_set_error(gs, GS_ERROR, "libusb_submit_transfer failed: %s",
                 libusb_error_name(usb_error));
    return;
  }

  ++gs->writes_pending;
  gs_clear_error(gs);
}
