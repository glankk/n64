#ifndef N64_PI_H
#define N64_PI_H

#include <stdint.h>

#define PI_STATUS_DMA_BUSY  0x01
#define PI_STATUS_IO_BUSY   0x02
#define PI_STATUS_ERROR     0x04
#define PI_STATUS_RESET     0x01
#define PI_STATUS_CLR_INTR  0x02

typedef struct
{
  uint32_t dram_addr;
  uint32_t cart_addr;
  uint32_t rd_len;
  uint32_t wr_len;
  uint32_t status;
} pi_regs_t;

#define pi_regs             (*(volatile pi_regs_t*)0xA4600000)

#endif
