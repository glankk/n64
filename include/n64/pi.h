#ifndef N64_PI_H
#define N64_PI_H
#include <stdint.h>

#define PI_STATUS_DMA_BUSY      0x01
#define PI_STATUS_IO_BUSY       0x02
#define PI_STATUS_ERROR         0x04
#define PI_STATUS_RESET         0x01
#define PI_STATUS_CLR_INTR      0x02

#define DEVICE_TYPE_CART        0
#define DEVICE_TYPE_BULK        1
#define DEVICE_TYPE_64DD        2
#define DEVICE_TYPE_SRAM        3
#define DEVICE_TYPE_INIT        7

#define PI_DOMAIN1              0
#define PI_DOMAIN2              1

#define OS_MESG_TYPE_LOOPBACK   10
#define OS_MESG_TYPE_DMAREAD    11
#define OS_MESG_TYPE_DMAWRITE   12
#define OS_MESG_TYPE_VRETRACE   13
#define OS_MESG_TYPE_COUNTER    14
#define OS_MESG_TYPE_EDMAREAD   15
#define OS_MESG_TYPE_EDMAWRITE  16

#define OS_MESG_PRI_NORMAL      0
#define OS_MESG_PRI_HIGH        1

#define OS_READ                 0
#define OS_WRITE                1
#define OS_OTHERS               2

typedef struct
{
  uint32_t        dram_addr;
  uint32_t        cart_addr;
  uint32_t        rd_len;
  uint32_t        wr_len;
  uint32_t        status;
} pi_regs_t;

typedef struct
{
  uint32_t        errStatus;
  void           *dramAddr;
  void           *C2Addr;
  uint32_t        sectorSize;
  uint32_t        C1ErrNum;
  uint32_t        C1ErrSector[4];
} __OSBlockInfo;

typedef struct
{
  uint32_t        cmdType;
  uint16_t        transferMode;
  uint16_t        blockNum;
  int32_t         sectorNum;
  uint32_t        devAddr;
  uint32_t        bmCtlShadow;
  uint32_t        seqCtlShadow;
  __OSBlockInfo   block[2];
} __OSTranxInfo;

typedef struct OSPiHandle_s OSPiHandle;
struct OSPiHandle_s
{
  OSPiHandle     *next;
  uint8_t         type;
  uint8_t         latency;
  uint8_t         pageSize;
  uint8_t         relDuration;
  uint8_t         pulse;
  uint8_t         domain;
  uint32_t        baseAddress;
  uint32_t        speed;
  __OSTranxInfo   transferInfo;
};

typedef struct
{
  uint16_t        type;
  uint8_t         pri;
  uint8_t         status;
  OSMesgQueue    *retQueue;
} OSIoMesgHdr;

typedef struct
{
  OSIoMesgHdr     hdr;
  void           *dramAddr;
  uint32_t        devAddr;
  uint32_t        size;
  OSPiHandle     *piHandle;
} OSIoMesg;

typedef int32_t (*osEPiStartDma_t)(OSPiHandle*, OSIoMesg*, int32_t);

#define pi_regs (*(volatile pi_regs_t*)0xA4600000)

#endif
