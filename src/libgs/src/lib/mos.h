#ifndef MOS_H
#define MOS_H

#define VENDOR_ID                     0x9710
#define PRODUCT_ID                    0x7705

#define REQTYPE_READ                  (LIBUSB_ENDPOINT_IN | \
                                       LIBUSB_TRANSFER_TYPE_CONTROL | \
                                       LIBUSB_REQUEST_TYPE_VENDOR)

#define REQTYPE_WRITE                 (LIBUSB_ENDPOINT_OUT | \
                                       LIBUSB_TRANSFER_TYPE_CONTROL | \
                                       LIBUSB_REQUEST_TYPE_VENDOR)

#define ENDPOINT_MOS_BULK_READ        0x81
#define ENDPOINT_MOS_BULK_WRITE       0x02

#define REQ_MOS_WRITE                 0x0E
#define REQ_MOS_READ                  0x0D

/* value field, only one port on the 7705 */
#define MOS_PP_PORT                   0
#define MOS_PORT_BASE                 ((MOS_PP_PORT + 1) << 8)

/* index field, simulated IBM PC interface */
#define MOS_PP_DATA_REG               0x0000
#define MOS_PP_STATUS_REG             0x0001
#define MOS_PP_CONTROL_REG            0x0002
#define MOS_PP_DEBUG_REG              0x0004
#define MOS_PP_EXTENDED_CONTROL_REG   0x000A

#define MOS_SPP_MODE                  (0 << 5)
#define MOS_NIBBLE_MODE               (1 << 5) /* default on reset */
#define MOS_FIFO_MODE                 (2 << 5)

#endif
