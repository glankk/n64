#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define MIPS_ASM_BIG_ENDIAN
#define MIPS_ASM_CONCISE
#include "mips.h"


void *generate_setup(uint32_t payload_entrypoint, uint32_t patch_addr)
{
  /* Embedded pre-setup code */
  uint32_t code[] =
  {
    /* Stop GameShark traps */
    MTC0(R0, MIPS_CP0_WATCHLO),
    NOP,

    /* Disable Interrupts */
    MFC0(T0, MIPS_CP0_SR),
    LI(T1, ~MIPS_STATUS_IE),
    AND(T0, T0, T1),
    MTC0(T0, MIPS_CP0_SR),

    /* Modify EPC */
    LA(K0, payload_entrypoint),
    MTC0(K0, MIPS_CP0_EPC),

    /* Patch back modified code handler */
    LA(K1, 0x3C1A8000),
    LA(K0, patch_addr),
    SW(K1, 0, K0),

    /* Halt RSP */
    LUI(T1, 2),
    LA(T0, 0xA4040010),
    SW(T1, 0, T0),

    /* Halt RDP */
    LUI(T1, 1|4|0x10|0x40|0x80|0x100|0x200),
    LA(T0, 0xA410000C),
    SW(T1, 0, T0),

    /* Return from interrupt - execute code */
    ERET,
    NOP,
  };
  void *p = malloc(sizeof(code));
  memcpy(p, code, sizeof(code));
  return p;
}

#define DEBOUNCE_COUNT 1
#define GS_READ_PORT   0x80787C88

void *generate_2x_receive()
{
  uint32_t code[] =
  {
    /* Function: fast (2x) receive byte */
    SUBIU(SP, SP, 0x28),
    SW(S0, 0x10, SP),
    SW(S1, 0x14, SP),
    SW(S2, 0x18, SP),
    SW(RA, 0x1C, SP),

    /* wait for consistent high nibble */
    ORI(S2, R0, DEBOUNCE_COUNT),
    JAL(GS_READ_PORT),
    NOP,
    ANDI(A0, V0, 0x10),
    BEQ(A0, R0, -4*4),
    NOP,
    BNE(S2, R0, -6*4),
    ADDIU(S2, S2, -1),

    /* collect the nibble */
    ANDI(S0, V0, 0xF),
    SLL(S0, S0, 4),

    /* wait for consistent low nibble */
    ORI(S2, R0, DEBOUNCE_COUNT),
    JAL(GS_READ_PORT),
    NOP,
    ANDI(A0, V0, 0x10),
    BNE(A0, R0, -4*4),
    NOP,
    BNE(S2, R0, -6*4),
    ADDIU(S2, S2, -1),

    /* collect the nibble */
    ANDI(V0, V0, 0xF),
    OR(S0, V0, S0),

    /* load return value */
    OR(V0, S0, R0),
    /* restore saved regs */
    LW(S0, 0x10, SP),
    LW(S1, 0x14, SP),
    LW(S2, 0x18, SP),
    LW(RA, 0x1C, SP),
    ADDIU(SP, SP, 0x28),
    JR(RA),
    NOP,
  };
  void *p = malloc(sizeof(code));
  memcpy(p, code, sizeof(code));
  return p;
}

#define ACK_PULSE_WIDTH   2
#define GS_WRITE_PORT     0x80787C24

/*              nERR  nBUSY nACK  */
/* 0x00 = 0x80  0     1     0     */
/* 0x10 = 0x88  1     1     0     */
/* 0x14 = 0xC8  1     1     1     */
/* 0x18 = 0x08  1     0     0     */
/* 0x1C = 0x48  1     0     1     */

#define SIMULATE_PROMPT \
  JAL(GS_WRITE_PORT), \
  ORI(A0, R0, 0x14)

#define SIMULATE_ACK \
  JAL(GS_WRITE_PORT), \
  ORI(A0, R0, 0x10),  \
  ORI(A0, R0, ACK_PULSE_WIDTH), \
  BNE(A0, R0, -1*4),  \
  ADDIU(A0, A0, -1),  \
  JAL(GS_WRITE_PORT), \
  ORI(A0, R0, 0x14)

#define ACK_AND_CLEANUP \
  JAL(GS_WRITE_PORT), \
  ORI(A0, R0, 0x10),  \
  ORI(A0, R0, ACK_PULSE_WIDTH), \
  BNE(A0, R0, -1*4),  \
  ADDIU(A0, A0, -1),  \
  JAL(GS_WRITE_PORT), \
  ORI(A0, R0, 0x00)

void *generate_bulk_receive()
{
  uint32_t code[] =
  {
    /* Function: bulk receive byte */
    SUBIU(SP, SP, 0x28),
    SW(S0, 0x10, SP),
    SW(S1, 0x14, SP),
    SW(S2, 0x18, SP),
    SW(RA, 0x1C, SP),

    SIMULATE_PROMPT,

    /* wait for consistent high nibble */
    JAL(GS_READ_PORT),
    NOP,
    ANDI(A0, V0, 0x10),
    BEQ(A0, R0, -4*4),
    NOP,

    JAL(GS_READ_PORT),
    NOP,
    ANDI(A0, V0, 0x10),
    BEQ(A0, R0, -9*4),
    ANDI(V0, V0, 0xF),

    /* collect the nibble */
    SLL(S0, V0, 4),

    SIMULATE_ACK,

    /* wait for consistent low nibble */
    JAL(GS_READ_PORT),
    NOP,
    ANDI(A0, V0, 0x10),
    BNE(A0, R0, -4*4),
    NOP,

    JAL(GS_READ_PORT),
    NOP,
    ANDI(A0, V0, 0x10),
    BNE(A0, R0, -9*4),
    ANDI(V0, V0, 0xF),

    /* collect the nibble */
    OR(S0, S0, V0),

    ACK_AND_CLEANUP,

    /* load return value */
    OR(V0, S0, R0),
    /* restore saved regs */
    LW(S0, 0x10, SP),
    LW(S1, 0x14, SP),
    LW(S2, 0x18, SP),
    LW(RA, 0x1C, SP),
    ADDIU(SP, SP, 0x28),
    JR(RA),
    NOP,
  };
  void *p = malloc(sizeof(code));
  memcpy(p, code, sizeof(code));
  return p;
}
