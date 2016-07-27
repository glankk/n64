/**
 * mips.h version 0.1rc4
 * nec vr4300, mips iii
 * note: providing arguments with side-effects to these macros should
 * be considered unsafe in general, as they may be evaluated more than once
 * -glank
**/

#ifndef MIPS_H
#define MIPS_H

#include <stddef.h>
#include <stdint.h>
#include <sys/param.h>

/**
 * private helper macros
**/

#if defined BYTE_ORDER && defined LITTLE_ENDIAN && defined BIG_ENDIAN
#if BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN
#define MIPS_BYTE_ORDER_UNSUPPORTED
#endif
#else
#define MIPS_BYTE_ORDER_UNSUPPORTED
#endif
#if defined MIPS_ASM_LITTLE_ENDIAN && defined MIPS_ASM_BIG_ENDIAN
#error conflicting endianness
#endif
#if (defined MIPS_ASM_LITTLE_ENDIAN || defined MIPS_ASM_BIG_ENDIAN) && \
    defined MIPS_BYTE_ORDER_UNSUPPORTED
#error byte order unsupported
#endif
#if (defined MIPS_ASM_BIG_ENDIAN && BYTE_ORDER == LITTLE_ENDIAN) || \
    (defined MIPS_ASM_LITTLE_ENDIAN && BYTE_ORDER == BIG_ENDIAN)
#define MIPS_BO_(x)               ((((x)&0x000000FF)<<24)| \
                                   (((x)&0x0000FF00)<<8) | \
                                   (((x)&0x00FF0000)>>8) | \
                                   (((x)&0xFF000000)>>24))
#else
#define MIPS_BO_(x)               (x)
#endif
#define MIPS_I_(x)                ((uint32_t)(x))
#define MIPS_OP_(x)               ((MIPS_I_(x)&0x3F)<<26)
#define MIPS_RS_(x)               ((MIPS_I_(x)&0x1F)<<21)
#define MIPS_RT_(x)               ((MIPS_I_(x)&0x1F)<<16)
#define MIPS_RD_(x)               ((MIPS_I_(x)&0x1F)<<11)
#define MIPS_SA_(x)               ((MIPS_I_(x)&0x1F)<<6)
#define MIPS_FN_(x)               (MIPS_I_(x)&0x3F)
#define MIPS_IM_(x)               (MIPS_I_(x)&0xFFFF)
#define MIPS_AD_(x)               ((MIPS_I_(x)&0xFFFFFFF)>>2)
#define MIPS_OF_(x)               ((MIPS_I_(x)>>2)&0xFFFF)
#define MIPS_INI_(op,rs,rt,im)    MIPS_BO_(MIPS_OP_(op)|MIPS_RS_(rs)| \
                                           MIPS_RT_(rt)|MIPS_IM_(im))
#define MIPS_INB_(op,rs,rt,of)    MIPS_BO_(MIPS_OP_(op)|MIPS_RS_(rs)| \
                                           MIPS_RT_(rt)|MIPS_OF_(of))
#define MIPS_INS_(fn,rs,rt,rd,sa) MIPS_BO_(MIPS_RS_(rs)|MIPS_RT_(rt)| \
                                           MIPS_RD_(rd)|MIPS_SA_(sa)| \
                                           MIPS_FN_(fn))
#define MIPS_INR_(fn,rs,rt,rd)    MIPS_INS_(fn,rs,rt,rd,0)
#define MIPS_INJ_(op,ad)          MIPS_BO_(MIPS_OP_(op)|MIPS_AD_(ad))
#define MIPS_INC_(fn,fm,rt,fs)    MIPS_BO_(MIPS_OP_(0x10)|MIPS_RS_(fm)| \
                                           MIPS_RT_(rt)|MIPS_RD_(fs)| \
                                           MIPS_FN_(fn))
#define MIPS_INF_(fn,fm,ft,fs,fd) MIPS_BO_(MIPS_OP_(0x11)|MIPS_RS_(fm)| \
                                           MIPS_RT_(ft)|MIPS_RD_(fs)| \
                                           MIPS_SA_(fd)|MIPS_FN_(fn))
#define MIPS_INFB_(nt,of)         MIPS_INB_(0x11,0x08,nt,of)
#define MIPS_FMS_                 0x10
#define MIPS_FMD_                 0x11
#define MIPS_FMW_                 0x14
#define MIPS_FML_                 0x15

/**
 * mips general
**/

/* field manipulation macros */
#define MIPS_GET_FIELD(f,x)       ((MIPS_I_(x)&f##MASK)>>f##SHIFT)
#define MIPS_MAKE_FIELD(f,x)      ((MIPS_I_(x)<<f##SHIFT)&f##MASK)
#define MIPS_CLEAR_FIELD(f,x)     (MIPS_I_(x)&~f##MASK)
#define MIPS_SET_FIELD(f,x,y)     (MIPS_CLEAR_FIELD(f,x)|MIPS_MAKE_FIELD(f,y))
#define MIPS_POSTMOD_FIELD(f,x,y) MIPS_SET_FIELD(f,x,MIPS_GET_FIELD(f,x) y)
#define MIPS_PREMOD_FIELD(f,y,x)  MIPS_SET_FIELD(f,x,y MIPS_GET_FIELD(f,x))

/* segments */
#define MIPS_KUSEG_BASE           MIPS_I_(0x00000000)
#define MIPS_KUSEG_SIZE           MIPS_I_(0x80000000)
#define MIPS_KSEG0_BASE           MIPS_I_(0x80000000)
#define MIPS_KSEG0_SIZE           MIPS_I_(0x20000000)
#define MIPS_KSEG1_BASE           MIPS_I_(0xA0000000)
#define MIPS_KSEG1_SIZE           MIPS_I_(0x20000000)
#define MIPS_KSSEG_BASE           MIPS_I_(0xC0000000)
#define MIPS_KSSEG_SIZE           MIPS_I_(0x20000000)
#define MIPS_KSEG3_BASE           MIPS_I_(0xE0000000)
#define MIPS_KSEG3_SIZE           MIPS_I_(0x20000000)
#define MIPS_SUSEG_BASE           MIPS_I_(0x00000000)
#define MIPS_SUSEG_SIZE           MIPS_I_(0x80000000)
#define MIPS_SSEG_BASE            MIPS_I_(0xC0000000)
#define MIPS_SSEG_SIZE            MIPS_I_(0x20000000)
#define MIPS_USEG_BASE            MIPS_I_(0x00000000)
#define MIPS_USEG_SIZE            MIPS_I_(0x80000000)

/* exception vectors */
#define MIPS_EVSIZE               0x80
#define MIPS_EV_RNMI              (MIPS_KSEG1_BASE+0x1FC00000)
#define MIPS_EV_TLBM              (MIPS_KSEG0_BASE+0x00000000)
#define MIPS_EV_XTLBM             (MIPS_KSEG0_BASE+0x00000080)
#define MIPS_EV_GE                (MIPS_KSEG0_BASE+0x00000180)

/* address conversion */
#define MIPS_KSEG0_TO_KSEG1(x)    (MIPS_I_(x)|0xA0000000)
#define MIPS_KSEG0_TO_PHYS(x)     (MIPS_I_(x)&0x1FFFFFFF)
#define MIPS_KSEG1_TO_KSEG0(x)    (MIPS_I_(x)&0x9FFFFFFF)
#define MIPS_KSEG1_TO_PHYS(x)     (MIPS_I_(x)&0x1FFFFFFF)
#define MIPS_KDM_TO_PHYS(x)       (MIPS_I_(x)&0x1FFFFFFF)
#define MIPS_PHYS_TO_KSEG0(x)     (MIPS_I_(x)|0x80000000)
#define MIPS_PHYS_TO_KSEG1(x)     (MIPS_I_(x)|0xA0000000)

/* address predicates */
#define MIPS_IS_KUSEG(x)          (MIPS_I_(x)>=MIPS_KUSEG_BASE && \
                                   MIPS_I_(x)<MIPS_KSEG0_BASE)
#define MIPS_IS_KSEG0(x)          (MIPS_I_(x)>=MIPS_KSEG0_BASE && \
                                   MIPS_I_(x)<MIPS_KSEG1_BASE)
#define MIPS_IS_KSEG1(x)          (MIPS_I_(x)>=MIPS_KSEG1_BASE && \
                                   MIPS_I_(x)<MIPS_KSSEG_BASE)
#define MIPS_IS_KDM(x)            (MIPS_I_(x)>=MIPS_KSEG0_BASE && \
                                   MIPS_I_(x)<MIPS_KSSEG_BASE)
#define MIPS_IS_KSSEG(x)          (MIPS_I_(x)>=MIPS_KSSEG_BASE && \
                                   MIPS_I_(x)<MIPS_KSEG3_BASE)
#define MIPS_IS_KSEG3(x)          (MIPS_I_(x)>=MIPS_KSEG3_BASE)
#define MIPS_IS_SUSEG             MIPS_IS_KUSEG
#define MIPS_IS_SSEG              MIPS_IS_KSSEG
#define MIPS_IS_USEG              MIPS_IS_KUSEG

/* status register */
#define MIPS_STATUS_CU3           MIPS_I_(0x80000000)
#define MIPS_STATUS_CU2           MIPS_I_(0x40000000)
#define MIPS_STATUS_CU1           MIPS_I_(0x20000000)
#define MIPS_STATUS_CU0           MIPS_I_(0x10000000)
#define MIPS_STATUS_RP            MIPS_I_(0x08000000)
#define MIPS_STATUS_FR            MIPS_I_(0x04000000)
#define MIPS_STATUS_RE            MIPS_I_(0x02000000)
#define MIPS_STATUS_ITS           MIPS_I_(0x01000000)
#define MIPS_STATUS_BEV           MIPS_I_(0x00400000)
#define MIPS_STATUS_TS            MIPS_I_(0x00200000)
#define MIPS_STATUS_SR            MIPS_I_(0x00100000)
#define MIPS_STATUS_CH            MIPS_I_(0x00040000)
#define MIPS_STATUS_CE            MIPS_I_(0x00020000)
#define MIPS_STATUS_DE            MIPS_I_(0x00010000)
#define MIPS_STATUS_IM7           MIPS_I_(0x00008000)
#define MIPS_STATUS_IM6           MIPS_I_(0x00004000)
#define MIPS_STATUS_IM5           MIPS_I_(0x00002000)
#define MIPS_STATUS_IM4           MIPS_I_(0x00001000)
#define MIPS_STATUS_IM3           MIPS_I_(0x00000800)
#define MIPS_STATUS_IM2           MIPS_I_(0x00000400)
#define MIPS_STATUS_IM1           MIPS_I_(0x00000200)
#define MIPS_STATUS_IM0           MIPS_I_(0x00000100)
#define MIPS_STATUS_KX            MIPS_I_(0x00000080)
#define MIPS_STATUS_SX            MIPS_I_(0x00000040)
#define MIPS_STATUS_UX            MIPS_I_(0x00000020)
#define MIPS_STATUS_KSUMASK       MIPS_I_(0x00000018)
#define MIPS_STATUS_KSUSHIFT      3
#define MIPS_STATUS_KSU_U         2
#define MIPS_STATUS_KSU_S         1
#define MIPS_STATUS_KSU_K         0
#define MIPS_STATUS_ERL           MIPS_I_(0x00000004)
#define MIPS_STATUS_EXL           MIPS_I_(0x00000002)
#define MIPS_STATUS_IE            MIPS_I_(0x00000001)

/* cause register */
#define MIPS_CAUSE_BD             MIPS_I_(0x80000000)
#define MIPS_CAUSE_CEMASK         MIPS_I_(0x30000000)
#define MIPS_CAUSE_CESHIFT        28
#define MIPS_CAUSE_IP7            MIPS_I_(0x00008000)
#define MIPS_CAUSE_IP6            MIPS_I_(0x00004000)
#define MIPS_CAUSE_IP5            MIPS_I_(0x00002000)
#define MIPS_CAUSE_IP4            MIPS_I_(0x00001000)
#define MIPS_CAUSE_IP3            MIPS_I_(0x00000800)
#define MIPS_CAUSE_IP2            MIPS_I_(0x00000400)
#define MIPS_CAUSE_IP1            MIPS_I_(0x00000200)
#define MIPS_CAUSE_IP0            MIPS_I_(0x00000100)
#define MIPS_CAUSE_EXCMASK        MIPS_I_(0x0000007C)
#define MIPS_CAUSE_EXCSHIFT       2

/* exception codes */
#define MIPS_EXC_INT              0
#define MIPS_EXC_MOD              1
#define MIPS_EXC_TLBL             2
#define MIPS_EXC_TLBS             3
#define MIPS_EXC_ADEL             4
#define MIPS_EXC_ADES             5
#define MIPS_EXC_IBE              6
#define MIPS_EXC_DBE              7
#define MIPS_EXC_SYS              8
#define MIPS_EXC_BP               9
#define MIPS_EXC_RI               10
#define MIPS_EXC_CPU              11
#define MIPS_EXC_OV               12
#define MIPS_EXC_TR               13
#define MIPS_EXC_FPE              15
#define MIPS_EXC_WATCH            23

/* prid */
#define MIPS_PRID_IMPMASK         MIPS_I_(0xFF00)
#define MIPS_PRID_IMPSHIFT        8
#define MIPS_PRID_REVMASK         MIPS_I_(0x00FF)
#define MIPS_PRID_REVSHIFT        0

/* cache targets */
#define MIPS_CACHEMASK            MIPS_I_(0x03)
#define MIPS_CACHESHIFT           0
#define MIPS_CACHE_I              0
#define MIPS_CACHE_D              1

/* cache operations */
#define MIPS_CACHEOPMASK          MIPS_I_(0x1C)
#define MIPS_CACHEOPSHIFT         2
#define MIPS_CACHEOP_II           0
#define MIPS_CACHEOP_IWBI         0
#define MIPS_CACHEOP_ILT          1
#define MIPS_CACHEOP_IST          2
#define MIPS_CACHEOP_CDE          3
#define MIPS_CACHEOP_HI           4
#define MIPS_CACHEOP_HWBI         5
#define MIPS_CACHEOP_F            5
#define MIPS_CACHEOP_HWB          6

/* config register */
#define MIPS_CONFIG_ECMASK        MIPS_I_(0x70000000)
#define MIPS_CONFIG_ECSHIFT       28
#define MIPS_CONFIG_EC_1_1        6
#define MIPS_CONFIG_EC_3_2        7
#define MIPS_CONFIG_EC_2_1        0
#define MIPS_CONFIG_EC_3_1        1
#define MIPS_CONFIG_EPMASK        MIPS_I_(0x0F000000)
#define MIPS_CONFIG_EPSHIFT       24
#define MIPS_CONFIG_EP_D          0
#define MIPS_CONFIG_EP_DXXDXX     6
#define MIPS_CONFIG_BE            MIPS_I_(0x00008000)
#define MIPS_CONFIG_K0MASK        MIPS_I_(0x00000007)
#define MIPS_CONFIG_K0SHIFT       0
#define MIPS_CONFIG_K0_NC         2
#define MIPS_CONFIG_K0_C          3

/* taglo register */
#define MIPS_TAGLO_PTLMASK        MIPS_I_(0x0FFFFF00)
#define MIPS_TAGLO_PTLSHIFT       8
#define MIPS_TAGLO_PSMASK         MIPS_I_(0x000000C0)
#define MIPS_TAGLO_PSSHIFT        6
#define MIPS_TAGLO_PS_INVD        0
#define MIPS_TAGLO_PS_DIRTY       3

/* memory breakpoints */
#define MIPS_WATCHLO_PAMASK       MIPS_I_(0xFFFFFFF8)
#define MIPS_WATCHLO_PASHIFT      3
#define MIPS_WATCHLO_R            MIPS_I_(0x00000002)
#define MIPS_WATCHLO_W            MIPS_I_(0x00000001)

/* cp0 registers */
#define MIPS_CP0_INDEX            0
#define MIPS_CP0_RANDOM           1
#define MIPS_CP0_ENTRYLO0         2
#define MIPS_CP0_ENTRYLO1         3
#define MIPS_CP0_CONTEXT          4
#define MIPS_CP0_PAGEMASK         5
#define MIPS_CP0_WIRED            6
#define MIPS_CP0_BADVADDR         8
#define MIPS_CP0_COUNT            9
#define MIPS_CP0_ENTRYHI          10
#define MIPS_CP0_COMPARE          11
#define MIPS_CP0_SR               12
#define MIPS_CP0_CAUSE            13
#define MIPS_CP0_EPC              14
#define MIPS_CP0_PRID             15
#define MIPS_CP0_CONFIG           16
#define MIPS_CP0_LLADDR           17
#define MIPS_CP0_WATCHLO          18
#define MIPS_CP0_WATCHHI          19
#define MIPS_CP0_XCONTEXT         20
#define MIPS_CP0_PERR             26
#define MIPS_CP0_CACHEERR         27
#define MIPS_CP0_TAGLO            28
#define MIPS_CP0_TAGHI            29
#define MIPS_CP0_ERREPC           30

/* floating point control registers */
#define MIPS_FCR_IR               0
#define MIPS_FCR_CS               31

/* floating point control and status register */
#define MIPS_FCSR_FS              MIPS_I_(0x01000000)
#define MIPS_FCSR_C               MIPS_I_(0x00800000)
#define MIPS_FCSR_CE              MIPS_I_(0x00020000)
#define MIPS_FCSR_CV              MIPS_I_(0x00010000)
#define MIPS_FCSR_CZ              MIPS_I_(0x00008000)
#define MIPS_FCSR_CO              MIPS_I_(0x00004000)
#define MIPS_FCSR_CU              MIPS_I_(0x00002000)
#define MIPS_FCSR_CI              MIPS_I_(0x00001000)
#define MIPS_FCSR_EV              MIPS_I_(0x00000800)
#define MIPS_FCSR_EZ              MIPS_I_(0x00000400)
#define MIPS_FCSR_EO              MIPS_I_(0x00000200)
#define MIPS_FCSR_EU              MIPS_I_(0x00000100)
#define MIPS_FCSR_EI              MIPS_I_(0x00000080)
#define MIPS_FCSR_FV              MIPS_I_(0x00000040)
#define MIPS_FCSR_FZ              MIPS_I_(0x00000020)
#define MIPS_FCSR_FO              MIPS_I_(0x00000010)
#define MIPS_FCSR_FU              MIPS_I_(0x00000008)
#define MIPS_FCSR_FI              MIPS_I_(0x00000004)
#define MIPS_FCSR_RMMASK          MIPS_I_(0x00000003)
#define MIPS_FCSR_RMSHIFT         0
#define MIPS_FCSR_RM_RN           0
#define MIPS_FCSR_RM_RZ           1
#define MIPS_FCSR_RM_RP           2
#define MIPS_FCSR_RM_RM           3

/**
 * inline assembly macros
**/

/* general purpose registers */
#define MIPS_R0                   0
#define MIPS_AT                   1
#define MIPS_V0                   2
#define MIPS_V1                   3
#define MIPS_A0                   4
#define MIPS_A1                   5
#define MIPS_A2                   6
#define MIPS_A3                   7
#define MIPS_T0                   8
#define MIPS_T1                   9
#define MIPS_T2                   10
#define MIPS_T3                   11
#define MIPS_T4                   12
#define MIPS_T5                   13
#define MIPS_T6                   14
#define MIPS_T7                   15
#define MIPS_S0                   16
#define MIPS_S1                   17
#define MIPS_S2                   18
#define MIPS_S3                   19
#define MIPS_S4                   20
#define MIPS_S5                   21
#define MIPS_S6                   22
#define MIPS_S7                   23
#define MIPS_T8                   24
#define MIPS_T9                   25
#define MIPS_K0                   26
#define MIPS_K1                   27
#define MIPS_GP                   28
#define MIPS_SP                   29
#define MIPS_FP                   30
#define MIPS_RA                   31

/* floating point registers */
#define MIPS_F0                   0
#define MIPS_F1                   1
#define MIPS_F2                   2
#define MIPS_F3                   3
#define MIPS_F4                   4
#define MIPS_F5                   5
#define MIPS_F6                   6
#define MIPS_F7                   7
#define MIPS_F8                   8
#define MIPS_F9                   9
#define MIPS_F10                  10
#define MIPS_F11                  11
#define MIPS_F12                  12
#define MIPS_F13                  13
#define MIPS_F14                  14
#define MIPS_F15                  15
#define MIPS_F16                  16
#define MIPS_F17                  17
#define MIPS_F18                  18
#define MIPS_F19                  19
#define MIPS_F20                  20
#define MIPS_F21                  21
#define MIPS_F22                  22
#define MIPS_F23                  23
#define MIPS_F24                  24
#define MIPS_F25                  25
#define MIPS_F26                  26
#define MIPS_F27                  27
#define MIPS_F28                  28
#define MIPS_F29                  29
#define MIPS_F30                  30
#define MIPS_F31                  31

/* loads and stores */
#define MIPS_LB(rt,of,rs)         MIPS_INI_(0x20,rs,rt,of)
#define MIPS_LBU(rt,of,rs)        MIPS_INI_(0x24,rs,rt,of)
#define MIPS_LD(rt,of,rs)         MIPS_INI_(0x37,rs,rt,of)
#define MIPS_LDL(rt,of,rs)        MIPS_INI_(0x1A,rs,rt,of)
#define MIPS_LDR(rt,of,rs)        MIPS_INI_(0x1B,rs,rt,of)
#define MIPS_LH(rt,of,rs)         MIPS_INI_(0x21,rs,rt,of)
#define MIPS_LHU(rt,of,rs)        MIPS_INI_(0x25,rs,rt,of)
#define MIPS_LL(rt,of,rs)         MIPS_INI_(0x30,rs,rt,of)
#define MIPS_LLD(rt,of,rs)        MIPS_INI_(0x34,rs,rt,of)
#define MIPS_LW(rt,of,rs)         MIPS_INI_(0x23,rs,rt,of)
#define MIPS_LWL(rt,of,rs)        MIPS_INI_(0x22,rs,rt,of)
#define MIPS_LWR(rt,of,rs)        MIPS_INI_(0x26,rs,rt,of)
#define MIPS_LWU(rt,of,rs)        MIPS_INI_(0x27,rs,rt,of)
#define MIPS_SB(rt,of,rs)         MIPS_INI_(0x28,rs,rt,of)
#define MIPS_SC(rt,of,rs)         MIPS_INI_(0x38,rs,rt,of)
#define MIPS_SCD(rt,of,rs)        MIPS_INI_(0x3C,rs,rt,of)
#define MIPS_SD(rt,of,rs)         MIPS_INI_(0x3F,rs,rt,of)
#define MIPS_SDL(rt,of,rs)        MIPS_INI_(0x2C,rs,rt,of)
#define MIPS_SDR(rt,of,rs)        MIPS_INI_(0x2D,rs,rt,of)
#define MIPS_SH(rt,of,rs)         MIPS_INI_(0x29,rs,rt,of)
#define MIPS_SW(rt,of,rs)         MIPS_INI_(0x2B,rs,rt,of)
#define MIPS_SWL(rt,of,rs)        MIPS_INI_(0x2A,rs,rt,of)
#define MIPS_SWR(rt,of,rs)        MIPS_INI_(0x2E,rs,rt,of)
#define MIPS_SYNC                 MIPS_INR_(0x0F,0,0,0)

/* integer operations */
#define MIPS_ADD(rd,rs,rt)        MIPS_INR_(0x20,rs,rt,rd)
#define MIPS_ADDI(rt,rs,im)       MIPS_INI_(0x08,rs,rt,im)
#define MIPS_ADDIU(rt,rs,im)      MIPS_INI_(0x09,rs,rt,im)
#define MIPS_ADDU(rd,rs,rt)       MIPS_INR_(0x21,rs,rt,rd)
#define MIPS_AND(rd,rs,rt)        MIPS_INR_(0x24,rs,rt,rd)
#define MIPS_ANDI(rt,rs,im)       MIPS_INI_(0x0C,rs,rt,im)
#define MIPS_DADD(rd,rs,rt)       MIPS_INR_(0x2C,rs,rt,rd)
#define MIPS_DADDI(rt,rs,im)      MIPS_INI_(0x18,rs,rt,im)
#define MIPS_DADDIU(rt,rs,im)     MIPS_INI_(0x19,rs,rt,im)
#define MIPS_DADDU(rd,rs,rt)      MIPS_INR_(0x2D,rs,rt,rd)
#define MIPS_DDIV(rs,rt)          MIPS_INR_(0x1E,rs,rt,0)
#define MIPS_DDIVU(rs,rt)         MIPS_INR_(0x1F,rs,rt,0)
#define MIPS_DIV(rs,rt)           MIPS_INR_(0x1A,rs,rt,0)
#define MIPS_DIVU(rs,rt)          MIPS_INR_(0x1B,rs,rt,0)
#define MIPS_DMULT(rs,rt)         MIPS_INR_(0x1C,rs,rt,0)
#define MIPS_DMULTU(rs,rt)        MIPS_INR_(0x1D,rs,rt,0)
#define MIPS_DSLL(rd,rt,sa)       MIPS_INS_(0x38,0,rt,rd,sa)
#define MIPS_DSLL32(rd,rt,sa)     MIPS_INS_(0x3C,0,rt,rd,sa)
#define MIPS_DSLLV(rd,rt,rs)      MIPS_INR_(0x14,rs,rt,rd)
#define MIPS_DSRA(rd,rt,sa)       MIPS_INS_(0x3B,0,rt,rd,sa)
#define MIPS_DSRA32(rd,rt,sa)     MIPS_INS_(0x3F,0,rt,rd,sa)
#define MIPS_DSRAV(rd,rt,rs)      MIPS_INR_(0x17,rs,rt,rd)
#define MIPS_DSRL(rd,rt,sa)       MIPS_INS_(0x3A,0,rt,rd,sa)
#define MIPS_DSRL32(rd,rt,sa)     MIPS_INS_(0x3E,0,rt,rd,sa)
#define MIPS_DSRLV(rd,rt,rs)      MIPS_INR_(0x16,rs,rt,rd)
#define MIPS_DSUB(rd,rs,rt)       MIPS_INR_(0x2E,rs,rt,rd)
#define MIPS_DSUBU(rd,rs,rt)      MIPS_INR_(0x2F,rs,rt,rd)
#define MIPS_LUI(rt,im)           MIPS_INI_(0x0F,0,rt,im)
#define MIPS_MFHI(rd)             MIPS_INR_(0x10,0,0,rd)
#define MIPS_MFLO(rd)             MIPS_INR_(0x12,0,0,rd)
#define MIPS_MTHI(rs)             MIPS_INR_(0x11,rs,0,0)
#define MIPS_MTLO(rs)             MIPS_INR_(0x13,rs,0,0)
#define MIPS_MULT(rs,rt)          MIPS_INR_(0x18,rs,rt,0)
#define MIPS_MULTU(rs,rt)         MIPS_INR_(0x19,rs,rt,0)
#define MIPS_NOR(rd,rs,rt)        MIPS_INR_(0x27,rs,rt,rd)
#define MIPS_OR(rd,rs,rt)         MIPS_INR_(0x25,rs,rt,rd)
#define MIPS_ORI(rt,rs,im)        MIPS_INI_(0x0D,rs,rt,im)
#define MIPS_SLL(rd,rt,sa)        MIPS_INS_(0x00,0,rt,rd,sa)
#define MIPS_SLLV(rd,rt,rs)       MIPS_INR_(0x04,rs,rt,rd)
#define MIPS_SLT(rd,rs,rt)        MIPS_INR_(0x2A,rs,rt,rd)
#define MIPS_SLTI(rt,rs,im)       MIPS_INI_(0x0A,rs,rt,im)
#define MIPS_SLTIU(rt,rs,im)      MIPS_INI_(0x0B,rs,rt,im)
#define MIPS_SLTU(rd,rs,rt)       MIPS_INR_(0x2B,rs,rt,rd)
#define MIPS_SRA(rd,rt,sa)        MIPS_INS_(0x03,0,rt,rd,sa)
#define MIPS_SRAV(rd,rt,rs)       MIPS_INR_(0x07,rs,rt,rd)
#define MIPS_SRL(rd,rt,sa)        MIPS_INS_(0x02,0,rt,rd,sa)
#define MIPS_SRLV(rd,rt,rs)       MIPS_INR_(0x06,rs,rt,rd)
#define MIPS_SUB(rd,rs,rt)        MIPS_INR_(0x22,rs,rt,rd)
#define MIPS_SUBU(rd,rs,rt)       MIPS_INR_(0x23,rs,rt,rd)
#define MIPS_XOR(rd,rs,rt)        MIPS_INR_(0x26,rs,rt,rd)
#define MIPS_XORI(rt,rs,im)       MIPS_INI_(0x0E,rs,rt,im)

/* branching */
#define MIPS_BEQ(rs,rt,of)        MIPS_INB_(0x04,rs,rt,of)
#define MIPS_BEQL(rs,rt,of)       MIPS_INB_(0x14,rs,rt,of)
#define MIPS_BGEZ(rs,of)          MIPS_INB_(0x01,rs,0x01,of)
#define MIPS_BGEZAL(rs,of)        MIPS_INB_(0x01,rs,0x11,of)
#define MIPS_BGEZALL(rs,of)       MIPS_INB_(0x01,rs,0x13,of)
#define MIPS_BGEZL(rs,of)         MIPS_INB_(0x01,rs,0x03,of)
#define MIPS_BGTZ(rs,of)          MIPS_INB_(0x07,rs,0x00,of)
#define MIPS_BGTZL(rs,of)         MIPS_INB_(0x17,rs,0x00,of)
#define MIPS_BLEZ(rs,of)          MIPS_INB_(0x06,rs,0x00,of)
#define MIPS_BLEZL(rs,of)         MIPS_INB_(0x16,rs,0x00,of)
#define MIPS_BLTZ(rs,of)          MIPS_INB_(0x01,rs,0x00,of)
#define MIPS_BLTZAL(rs,of)        MIPS_INB_(0x01,rs,0x10,of)
#define MIPS_BLTZALL(rs,of)       MIPS_INB_(0x01,rs,0x12,of)
#define MIPS_BLTZL(rs,of)         MIPS_INB_(0x01,rs,0x02,of)
#define MIPS_BNE(rs,rt,of)        MIPS_INB_(0x05,rs,rt,of)
#define MIPS_BNEL(rs,rt,of)       MIPS_INB_(0x15,rs,rt,of)
#define MIPS_J(ad)                MIPS_INJ_(0x02,ad)
#define MIPS_JAL(ad)              MIPS_INJ_(0x03,ad)
#define MIPS_JALR(rd,rs)          MIPS_INR_(0x09,rs,0,rd)
#define MIPS_JR(rs)               MIPS_INR_(0x08,rs,0,0)

/* exceptions */
#define MIPS_BREAK                MIPS_INR_(0x0D,0,0,0)
#define MIPS_SYSCALL              MIPS_INR_(0x0C,0,0,0)
#define MIPS_TEQ(rs,rt)           MIPS_INR_(0x34,rs,rt,0)
#define MIPS_TEQI(rs,im)          MIPS_INI_(0x01,rs,0x0C,im)
#define MIPS_TGE(rs,rt)           MIPS_INR_(0x30,rs,rt,0)
#define MIPS_TGEI(rs,im)          MIPS_INI_(0x01,rs,0x08,im)
#define MIPS_TGEIU(rs,im)         MIPS_INI_(0x01,rs,0x09,im)
#define MIPS_TGEU(rs,rt)          MIPS_INR_(0x31,rs,rt,0)
#define MIPS_TLT(rs,rt)           MIPS_INR_(0x32,rs,rt,0)
#define MIPS_TLTI(rs,im)          MIPS_INI_(0x01,rs,0x0A,im)
#define MIPS_TLTIU(rs,im)         MIPS_INI_(0x01,rs,0x0B,im)
#define MIPS_TLTU(rs,rt)          MIPS_INR_(0x33,rs,rt,0)
#define MIPS_TNE(rs,rt)           MIPS_INR_(0x36,rs,rt,0)
#define MIPS_TNEI(rs,im)          MIPS_INI_(0x01,rs,0x0E,im)

/* system control */
#define MIPS_CACHE(op,of,rs)      MIPS_INI_(0x2F,rs,op,of)
#define MIPS_ERET                 MIPS_INC_(0x18,0x10,0,0)
#define MIPS_MFC0(rt,fs)          MIPS_INC_(0x00,0x00,rt,fs)
#define MIPS_MTC0(rt,fs)          MIPS_INC_(0x00,0x04,rt,fs)
#define MIPS_TLBP                 MIPS_INC_(0x08,0x10,0,0)
#define MIPS_TLBR                 MIPS_INC_(0x01,0x10,0,0)
#define MIPS_TLBWI                MIPS_INC_(0x02,0x10,0,0)
#define MIPS_TLBWR                MIPS_INC_(0x06,0x10,0,0)

/* floating point operations */
#define MIPS_ABSD(fd,fs)          MIPS_INF_(0x05,MIPS_FMD_,0,fs,fd)
#define MIPS_ABSS(fd,fs)          MIPS_INF_(0x05,MIPS_FMS_,0,fs,fd)
#define MIPS_ADDD(fd,fs,ft)       MIPS_INF_(0x00,MIPS_FMD_,ft,fs,fd)
#define MIPS_ADDS(fd,fs,ft)       MIPS_INF_(0x00,MIPS_FMS_,ft,fs,fd)
#define MIPS_BC1F(of)             MIPS_INFB_(0x00,of)
#define MIPS_BC1FL(of)            MIPS_INFB_(0x02,of)
#define MIPS_BC1T(of)             MIPS_INFB_(0x01,of)
#define MIPS_BC1TL(of)            MIPS_INFB_(0x03,of)
#define MIPS_CEILLD(fd,fs)        MIPS_INF_(0x0A,MIPS_FMD_,0,fs,fd)
#define MIPS_CEILLS(fd,fs)        MIPS_INF_(0x0A,MIPS_FMS_,0,fs,fd)
#define MIPS_CEILWD(fd,fs)        MIPS_INF_(0x0E,MIPS_FMD_,0,fs,fd)
#define MIPS_CEILWS(fd,fs)        MIPS_INF_(0x0E,MIPS_FMS_,0,fs,fd)
#define MIPS_CEQD(fs,ft)          MIPS_INF_(0x32,MIPS_FMD_,ft,fs,0)
#define MIPS_CEQS(fs,ft)          MIPS_INF_(0x32,MIPS_FMS_,ft,fs,0)
#define MIPS_CFC1(rt,fs)          MIPS_INF_(0x00,0x02,rt,fs,0)
#define MIPS_CFD(fs,ft)           MIPS_INF_(0x30,MIPS_FMD_,ft,fs,0)
#define MIPS_CFS(fs,ft)           MIPS_INF_(0x30,MIPS_FMS_,ft,fs,0)
#define MIPS_CLED(fs,ft)          MIPS_INF_(0x3E,MIPS_FMD_,ft,fs,0)
#define MIPS_CLES(fs,ft)          MIPS_INF_(0x3E,MIPS_FMS_,ft,fs,0)
#define MIPS_CLTD(fs,ft)          MIPS_INF_(0x3C,MIPS_FMD_,ft,fs,0)
#define MIPS_CLTS(fs,ft)          MIPS_INF_(0x3C,MIPS_FMS_,ft,fs,0)
#define MIPS_CNGED(fs,ft)         MIPS_INF_(0x3D,MIPS_FMD_,ft,fs,0)
#define MIPS_CNGES(fs,ft)         MIPS_INF_(0x3D,MIPS_FMS_,ft,fs,0)
#define MIPS_CNGLD(fs,ft)         MIPS_INF_(0x3B,MIPS_FMD_,ft,fs,0)
#define MIPS_CNGLED(fs,ft)        MIPS_INF_(0x39,MIPS_FMD_,ft,fs,0)
#define MIPS_CNGLES(fs,ft)        MIPS_INF_(0x39,MIPS_FMS_,ft,fs,0)
#define MIPS_CNGLS(fs,ft)         MIPS_INF_(0x3B,MIPS_FMS_,ft,fs,0)
#define MIPS_CNGTD(fs,ft)         MIPS_INF_(0x3F,MIPS_FMD_,ft,fs,0)
#define MIPS_CNGTS(fs,ft)         MIPS_INF_(0x3F,MIPS_FMS_,ft,fs,0)
#define MIPS_COLED(fs,ft)         MIPS_INF_(0x36,MIPS_FMD_,ft,fs,0)
#define MIPS_COLES(fs,ft)         MIPS_INF_(0x36,MIPS_FMS_,ft,fs,0)
#define MIPS_COLTD(fs,ft)         MIPS_INF_(0x34,MIPS_FMD_,ft,fs,0)
#define MIPS_COLTS(fs,ft)         MIPS_INF_(0x34,MIPS_FMS_,ft,fs,0)
#define MIPS_CSEQD(fs,ft)         MIPS_INF_(0x3A,MIPS_FMD_,ft,fs,0)
#define MIPS_CSEQS(fs,ft)         MIPS_INF_(0x3A,MIPS_FMS_,ft,fs,0)
#define MIPS_CSFD(fs,ft)          MIPS_INF_(0x38,MIPS_FMD_,ft,fs,0)
#define MIPS_CSFS(fs,ft)          MIPS_INF_(0x38,MIPS_FMS_,ft,fs,0)
#define MIPS_CTC1(rt,fs)          MIPS_INF_(0x00,0x06,rt,fs,0)
#define MIPS_CUEQD(fs,ft)         MIPS_INF_(0x33,MIPS_FMD_,ft,fs,0)
#define MIPS_CUEQS(fs,ft)         MIPS_INF_(0x33,MIPS_FMS_,ft,fs,0)
#define MIPS_CULED(fs,ft)         MIPS_INF_(0x37,MIPS_FMD_,ft,fs,0)
#define MIPS_CULES(fs,ft)         MIPS_INF_(0x37,MIPS_FMS_,ft,fs,0)
#define MIPS_CULTD(fs,ft)         MIPS_INF_(0x35,MIPS_FMD_,ft,fs,0)
#define MIPS_CULTS(fs,ft)         MIPS_INF_(0x35,MIPS_FMS_,ft,fs,0)
#define MIPS_CUND(fs,ft)          MIPS_INF_(0x31,MIPS_FMD_,ft,fs,0)
#define MIPS_CUNS(fs,ft)          MIPS_INF_(0x31,MIPS_FMS_,ft,fs,0)
#define MIPS_CVTDL(fd,fs)         MIPS_INF_(0x21,MIPS_FML_,0,fs,fd)
#define MIPS_CVTDS(fd,fs)         MIPS_INF_(0x21,MIPS_FMS_,0,fs,fd)
#define MIPS_CVTDW(fd,fs)         MIPS_INF_(0x21,MIPS_FMW_,0,fs,fd)
#define MIPS_CVTLD(fd,fs)         MIPS_INF_(0x25,MIPS_FMD_,0,fs,fd)
#define MIPS_CVTLS(fd,fs)         MIPS_INF_(0x25,MIPS_FMS_,0,fs,fd)
#define MIPS_CVTSD(fd,fs)         MIPS_INF_(0x20,MIPS_FMD_,0,fs,fd)
#define MIPS_CVTSL(fd,fs)         MIPS_INF_(0x20,MIPS_FML_,0,fs,fd)
#define MIPS_CVTSW(fd,fs)         MIPS_INF_(0x20,MIPS_FMW_,0,fs,fd)
#define MIPS_CVTWD(fd,fs)         MIPS_INF_(0x24,MIPS_FMD_,0,fs,fd)
#define MIPS_CVTWS(fd,fs)         MIPS_INF_(0x24,MIPS_FMS_,0,fs,fd)
#define MIPS_DIVD(fd,fs,ft)       MIPS_INF_(0x03,MIPS_FMD_,ft,fs,fd)
#define MIPS_DIVS(fd,fs,ft)       MIPS_INF_(0x03,MIPS_FMS_,ft,fs,fd)
#define MIPS_DMFC1(rt,fs)         MIPS_INF_(0x00,0x01,rt,fs,0)
#define MIPS_DMTC1(rt,fs)         MIPS_INF_(0x00,0x05,rt,fs,0)
#define MIPS_FLOORLD(fd,fs)       MIPS_INF_(0x0B,MIPS_FMD_,0,fs,fd)
#define MIPS_FLOORLS(fd,fs)       MIPS_INF_(0x0B,MIPS_FMS_,0,fs,fd)
#define MIPS_FLOORWD(fd,fs)       MIPS_INF_(0x0F,MIPS_FMD_,0,fs,fd)
#define MIPS_FLOORWS(fd,fs)       MIPS_INF_(0x0F,MIPS_FMS_,0,fs,fd)
#define MIPS_LDC1(ft,of,rs)       MIPS_INI_(0x35,rs,ft,of)
#define MIPS_LWC1(ft,of,rs)       MIPS_INI_(0x31,rs,ft,of)
#define MIPS_MFC1(rt,fs)          MIPS_INF_(0x00,0x00,rt,fs,0)
#define MIPS_MTC1(rt,fs)          MIPS_INF_(0x00,0x04,rt,fs,0)
#define MIPS_MOVD(fd,fs)          MIPS_INF_(0x06,MIPS_FMD_,0,fs,fd)
#define MIPS_MOVS(fd,fs)          MIPS_INF_(0x06,MIPS_FMS_,0,fs,fd)
#define MIPS_MULD(fd,fs,ft)       MIPS_INF_(0x02,MIPS_FMD_,ft,fs,fd)
#define MIPS_MULS(fd,fs,ft)       MIPS_INF_(0x02,MIPS_FMS_,ft,fs,fd)
#define MIPS_NEGD(fd,fs)          MIPS_INF_(0x07,MIPS_FMD_,0,fs,fd)
#define MIPS_NEGS(fd,fs)          MIPS_INF_(0x07,MIPS_FMS_,0,fs,fd)
#define MIPS_ROUNDLD(fd,fs)       MIPS_INF_(0x08,MIPS_FMD_,0,fs,fd)
#define MIPS_ROUNDLS(fd,fs)       MIPS_INF_(0x08,MIPS_FMS_,0,fs,fd)
#define MIPS_ROUNDWD(fd,fs)       MIPS_INF_(0x0C,MIPS_FMD_,0,fs,fd)
#define MIPS_ROUNDWS(fd,fs)       MIPS_INF_(0x0C,MIPS_FMS_,0,fs,fd)
#define MIPS_SDC1(ft,of,rs)       MIPS_INI_(0x3D,rs,ft,of)
#define MIPS_SQRTD(fd,fs)         MIPS_INF_(0x04,MIPS_FMD_,0,fs,fd)
#define MIPS_SQRTS(fd,fs)         MIPS_INF_(0x04,MIPS_FMS_,0,fs,fd)
#define MIPS_SUBD(fd,fs,ft)       MIPS_INF_(0x01,MIPS_FMD_,ft,fs,fd)
#define MIPS_SUBS(fd,fs,ft)       MIPS_INF_(0x01,MIPS_FMS_,ft,fs,fd)
#define MIPS_SWC1(ft,of,rs)       MIPS_INI_(0x39,rs,ft,of)
#define MIPS_TRUNCLD(fd,fs)       MIPS_INF_(0x09,MIPS_FMD_,0,fs,fd)
#define MIPS_TRUNCLS(fd,fs)       MIPS_INF_(0x09,MIPS_FMS_,0,fs,fd)
#define MIPS_TRUNCWD(fd,fs)       MIPS_INF_(0x0D,MIPS_FMD_,0,fs,fd)
#define MIPS_TRUNCWS(fd,fs)       MIPS_INF_(0x0D,MIPS_FMS_,0,fs,fd)

/* pseudo-instructions */
#define MIPS_ABS(rd,rs)           MIPS_BGEZ(rs,0x0008), \
                                  MIPS_OR(rd,rs,MIPS_R0), \
                                  MIPS_SUB(rd,MIPS_R0,rs)
#define MIPS_B(of)                MIPS_BEQ(MIPS_R0,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BAL(of)              MIPS_BGEZAL(MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BEQZ(rs,of)          MIPS_BEQ(rs,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BEQZL(rs,of)         MIPS_BEQL(rs,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGEI(rs1,is2,of)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGER(rs1,MIPS_AT,of)
#define MIPS_BGELI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGELR(rs1,MIPS_AT,of)
#define MIPS_BGELR(rs1,rs2,of)    MIPS_SLT(MIPS_AT,rs1,rs2), \
                                  MIPS_BEQL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGELUI(rs1,is2,of)   MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGELUR(rs1,MIPS_AT,of)
#define MIPS_BGELUR(rs1,rs2,of)   MIPS_SLTU(MIPS_AT,rs1,rs2), \
                                  MIPS_BEQL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGER(rs1,rs2,of)     MIPS_SLT(MIPS_AT,rs1,rs2), \
                                  MIPS_BEQ(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGEUI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGEUR(rs1,MIPS_AT,of)
#define MIPS_BGEUR(rs1,rs2,of)    MIPS_SLTU(MIPS_AT,rs1,rs2), \
                                  MIPS_BEQ(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGTI(rs1,is2,of)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGTR(rs1,MIPS_AT,of)
#define MIPS_BGTLI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGTLR(rs1,MIPS_AT,of)
#define MIPS_BGTLR(rs1,rs2,of)    MIPS_SLT(MIPS_AT,rs2,rs1), \
                                  MIPS_BNEL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGTLUI(rs1,is2,of)   MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGTLUR(rs1,MIPS_AT,of)
#define MIPS_BGTLUR(rs1,rs2,of)   MIPS_SLTU(MIPS_AT,rs2,rs1), \
                                  MIPS_BNEL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGTR(rs1,rs2,of)     MIPS_SLT(MIPS_AT,rs2,rs1), \
                                  MIPS_BNE(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BGTUI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BGTUR(rs1,MIPS_AT,of)
#define MIPS_BGTUR(rs1,rs2,of)    MIPS_SLTU(MIPS_AT,rs2,rs1), \
                                  MIPS_BNE(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLEI(rs1,is2,of)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLER(rs1,MIPS_AT,of)
#define MIPS_BLELI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLELR(rs1,MIPS_AT,of)
#define MIPS_BLELR(rs1,rs2,of)    MIPS_SLT(MIPS_AT,rs2,rs1), \
                                  MIPS_BEQL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLELUI(rs1,is2,of)   MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLELUR(rs1,MIPS_AT,of)
#define MIPS_BLELUR(rs1,rs2,of)   MIPS_SLTU(MIPS_AT,rs2,rs1), \
                                  MIPS_BEQL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLER(rs1,rs2,of)     MIPS_SLT(MIPS_AT,rs2,rs1), \
                                  MIPS_BEQ(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLEUI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLEUR(rs1,MIPS_AT,of)
#define MIPS_BLEUR(rs1,rs2,of)    MIPS_SLTU(MIPS_AT,rs2,rs1), \
                                  MIPS_BEQ(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLTI(rs1,is2,of)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLTR(rs1,MIPS_AT,of)
#define MIPS_BLTLI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLTLR(rs1,MIPS_AT,of)
#define MIPS_BLTLR(rs1,rs2,of)    MIPS_SLT(MIPS_AT,rs1,rs2), \
                                  MIPS_BNEL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLTLUI(rs1,is2,of)   MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLTLUR(rs1,MIPS_AT,of)
#define MIPS_BLTLUR(rs1,rs2,of)   MIPS_SLTU(MIPS_AT,rs1,rs2), \
                                  MIPS_BNEL(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLTR(rs1,rs2,of)     MIPS_SLT(MIPS_AT,rs1,rs2), \
                                  MIPS_BNE(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BLTUI(rs1,is2,of)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_BLTUR(rs1,MIPS_AT,of)
#define MIPS_BLTUR(rs1,rs2,of)    MIPS_SLTU(MIPS_AT,rs1,rs2), \
                                  MIPS_BNE(MIPS_AT,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BNEZ(rs,of)          MIPS_BNE(rs,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_BNEZL(rs,of)         MIPS_BNEL(rs,MIPS_R0,MIPS_I_(of)+0x04), \
                                  MIPS_NOP
#define MIPS_DIVR(rd,rs1,rs2)     MIPS_BNE(rs2,MIPS_R0,0x0008), \
                                  MIPS_DIV(rs1,rs2), \
                                  MIPS_BREAK, \
                                  MIPS_ADDIU(MIPS_AT,MIPS_R0,0xFFFF), \
                                  MIPS_BNE(rs2,MIPS_AT,0x0010), \
                                  MIPS_LUI(MIPS_AT,0x8000), \
                                  MIPS_BNE(rs1,MIPS_AT,0x0008), \
                                  MIPS_NOP, \
                                  MIPS_BREAK, \
                                  MIPS_MFLO(rd), \
                                  MIPS_NOP, \
                                  MIPS_NOP
#define MIPS_DIVUI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_DIVUR(rd,rs1,MIPS_AT)
#define MIPS_DIVUIR(rd,is1,rs2)   MIPS_LA(MIPS_AT,is1), \
                                  MIPS_DIVUR(rd,MIPS_AT,rs2)
#define MIPS_DIVUR(rd,rs1,rs2)    MIPS_BNE(rs2,MIPS_R0,0x008), \
                                  MIPS_DIVU(rs1,rs2), \
                                  MIPS_BREAK, \
                                  MIPS_MFLO(rd), \
                                  MIPS_NOP, \
                                  MIPS_NOP
#define MIPS_LA(rd,ad)            MIPS_LUI(rd,(MIPS_I_(ad)>>16)&0xFFFF), \
                                  MIPS_ORI(rd,rd,MIPS_I_(ad)&0xFFFF)
#define MIPS_LI(rd,is)            MIPS_ADDIU(rd,MIPS_R0,is)
#define MIPS_MOVE(rd,rs)          MIPS_OR(rd,rs,MIPS_R0)
#define MIPS_MULI(rd,rs1,is2)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_MULR(rd,rs1,MIPS_AT)
#define MIPS_MULOI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_MULOR(rd,rs1,MIPS_AT)
#define MIPS_MULOR(rd,rs1,rs2)    MIPS_MULT(rs1,rs2), \
                                  MIPS_MFLO(rd), \
                                  MIPS_SRA(rd,rd,0x1F), \
                                  MIPS_MFHI(MIPS_AT), \
                                  MIPS_BEQ(rd,MIPS_AT,0x0008), \
                                  MIPS_NOP, \
                                  MIPS_BREAK, \
                                  MIPS_MFLO(rd), \
                                  MIPS_NOP, \
                                  MIPS_NOP
#define MIPS_MULOUI(rd,rs1,is2)   MIPS_LA(MIPS_AT,is2), \
                                  MIPS_MULOUR(rd,rs1,MIPS_AT)
#define MIPS_MULOUR(rd,rs1,rs2)   MIPS_MULTU(rs1,rs2), \
                                  MIPS_MFHI(MIPS_AT), \
                                  MIPS_MFLO(rd), \
                                  MIPS_BEQ(MIPS_AT,MIPS_R0,0x0008), \
                                  MIPS_NOP, \
                                  MIPS_BREAK
#define MIPS_MULR(rd,rs1,rs2)     MIPS_MULTU(rs1,rs2), \
                                  MIPS_MFLO(rd), \
                                  MIPS_NOP, \
                                  MIPS_NOP
#define MIPS_NEG(rd,rs)           MIPS_SUB(rd,MIPS_R0,rs)
#define MIPS_NEGU(rd,rs)          MIPS_SUBU(rd,MIPS_R0,rs)
#define MIPS_NOP                  MIPS_SLL(MIPS_R0,MIPS_R0,0)
#define MIPS_NOT(rd,rs)           MIPS_NOR(rd,rs,MIPS_R0)
#define MIPS_REMR(rd,rs1,rs2)     MIPS_BNE(rs2,MIPS_R0,0x0008), \
                                  MIPS_DIV(rs1,rs2), \
                                  MIPS_BREAK, \
                                  MIPS_ADDIU(MIPS_AT,MIPS_R0,0xFFFF), \
                                  MIPS_BNE(rs2,MIPS_AT,0x0010), \
                                  MIPS_LUI(MIPS_AT,0x8000), \
                                  MIPS_BNE(rs1,MIPS_AT,0x0008), \
                                  MIPS_NOP, \
                                  MIPS_BREAK, \
                                  MIPS_MFHI(rd), \
                                  MIPS_NOP, \
                                  MIPS_NOP
#define MIPS_REMUI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_REMUR(rd,rs1,MIPS_AT)
#define MIPS_REMUIR(rd,is1,rs2)   MIPS_LA(MIPS_AT,is1), \
                                  MIPS_REMUR(rd,MIPS_AT,rs2)
#define MIPS_REMUR(rd,rs1,rs2)    MIPS_BNE(rs2,MIPS_R0,0x0008), \
                                  MIPS_DIVU(rs1,rs2), \
                                  MIPS_BREAK, \
                                  MIPS_MFHI(rd), \
                                  MIPS_NOP, \
                                  MIPS_NOP
#define MIPS_ROLI(rd,rs1,is2)     MIPS_SLL(MIPS_AT,rs1,MIPS_I_(is2)%0x20), \
                                  MIPS_SRL(rd,rs1,0x20-MIPS_I_(is2)%0x20), \
                                  MIPS_OR(rd,rd,MIPS_AT)
#define MIPS_ROLR(rd,rs1,rs2)     MIPS_SUBU(MIPS_AT,MIPS_R0,rs2), \
                                  MIPS_SRLV(MIPS_AT,rs1,MIPS_AT), \
                                  MIPS_SLLV(rd,rs1,rs2), \
                                  MIPS_OR(rd,rd,MIPS_AT)
#define MIPS_RORI(rd,rs1,is2)     MIPS_SRL(MIPS_AT,rs1,MIPS_I_(is2)%0x20), \
                                  MIPS_SLL(rd,rs1,0x20-MIPS_I_(is2)%0x20), \
                                  MIPS_OR(rd,rd,MIPS_AT)
#define MIPS_RORR(rd,rs1,rs2)     MIPS_SUBU(MIPS_AT,MIPS_R0,rs2), \
                                  MIPS_SLLV(MIPS_AT,rs1,MIPS_AT), \
                                  MIPS_SRLV(rd,rs1,rs2), \
                                  MIPS_OR(rd,rd,MIPS_AT)
#define MIPS_SEQI(rd,rs1,is2)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SEQR(rd,rs1,MIPS_AT)
#define MIPS_SEQR(rd,rs1,rs2)     MIPS_XOR(rd,rs1,rs2), \
                                  MIPS_SLTIU(rd,rd,0x0001)
#define MIPS_SGEI(rd,rs1,is2)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SGER(rd,rs1,MIPS_AT)
#define MIPS_SGER(rd,rs1,rs2)     MIPS_SLT(rd,rs1,rs2), \
                                  MIPS_XORI(rd,rd,0x0001)
#define MIPS_SGEUI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SGEUR(rd,rs1,MIPS_AT)
#define MIPS_SGEUR(rd,rs1,rs2)    MIPS_SLTU(rd,rs1,rs2), \
                                  MIPS_XORI(rd,rd,0x0001)
#define MIPS_SGTI(rd,rs1,is2)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SGTR(rd,rs1,MIPS_AT)
#define MIPS_SGTR(rd,rs1,rs2)     MIPS_SLT(rd,rs2,rs1)
#define MIPS_SGTUI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SGTUR(rd,rs1,MIPS_AT)
#define MIPS_SGTUR(rd,rs1,rs2)    MIPS_SLTU(rd,rs2,rs1)
#define MIPS_SLEI(rd,rs1,is2)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SLER(rd,rs1,MIPS_AT)
#define MIPS_SLER(rd,rs1,rs2)     MIPS_SLT(rd,rs2,rs1), \
                                  MIPS_XORI(rd,rd,0x0001)
#define MIPS_SLEUI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SLEUR(rd,rs1,MIPS_AT)
#define MIPS_SLEUR(rd,rs1,rs2)    MIPS_SLTU(rd,rs2,rs1), \
                                  MIPS_XORI(rd,rd,0x0001)
#define MIPS_SNEI(rd,rs1,is2)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SNER(rd,rs1,MIPS_AT)
#define MIPS_SNER(rd,rs1,rs2)     MIPS_XOR(rd,rs1,rs2), \
                                  MIPS_SLTU(rd,MIPS_R0,rd)
#define MIPS_SUBI(rd,rs1,is2)     MIPS_ADDI(rd,rs1,~MIPS_I_(is2)+1)
#define MIPS_SUBIR(rd,is1,rs2)    MIPS_NEG(rd,rs2), \
                                  MIPS_ADDI(rd,rd,is1)
#define MIPS_SUBIU(rd,rs1,is2)    MIPS_ADDIU(rd,rs1,~MIPS_I_(is2)+1)
#define MIPS_SUBIUR(rd,is1,rs2)   MIPS_NEGU(rd,rs2), \
                                  MIPS_ADDI(rd,rd,is1)
#define MIPS_ULH(rd,of,rs)        MIPS_LB(MIPS_AT,MIPS_I_(of)+0x00,rs), \
                                  MIPS_LBU(rd,MIPS_I_(of)+0x01,rs), \
                                  MIPS_SLL(MIPS_AT,MIPS_AT,0x08), \
                                  MIPS_OR(rd,rd,MIPS_AT)
#define MIPS_ULHU(rd,of,rs)       MIPS_LBU(MIPS_AT,MIPS_I_(of)+0x00,rs), \
                                  MIPS_LBU(rd,MIPS_I_(of)+0x01,rs), \
                                  MIPS_SLL(MIPS_AT,MIPS_AT,0x08), \
                                  MIPS_OR(rd,rd,MIPS_AT)
#define MIPS_ULW(rd,of,rs)        MIPS_LWL(rd,MIPS_I_(of)+0x00,rs), \
                                  MIPS_LWR(rd,MIPS_I_(of)+0x03,rs)
#define MIPS_USH(rs,of,rd)        MIPS_SB(rs,MIPS_I_(of)+0x01,rd), \
                                  MIPS_SRL(MIPS_AT,rs,0x08), \
                                  MIPS_SB(MIPS_AT,MIPS_I_(of)+0x00,rd)
#define MIPS_USW(rs,of,rd)        MIPS_SWL(rs,MIPS_I_(of)+0x00,rd), \
                                  MIPS_SWR(rs,MIPS_I_(of)+0x03,rd)
#define MIPS_WADDI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_ADD(rd,rs1,MIPS_AT)
#define MIPS_WADDIU(rd,rs1,is2)   MIPS_LA(MIPS_AT,is2), \
                                  MIPS_ADDU(rd,rs1,MIPS_AT)
#define MIPS_WANDI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_AND(rd,rs1,MIPS_AT)
#define MIPS_WORI(rd,rs1,is2)     MIPS_LA(MIPS_AT,is2), \
                                  MIPS_OR(rd,rs1,MIPS_AT)
#define MIPS_WSUBI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SUB(rd,rs1,MIPS_AT)
#define MIPS_WSUBIR(rd,is1,rs2)   MIPS_LA(MIPS_AT,is1), \
                                  MIPS_SUB(rd,MIPS_AT,rs2)
#define MIPS_WSUBIU(rd,rs1,is2)   MIPS_LA(MIPS_AT,is2), \
                                  MIPS_SUBU(rd,rs1,MIPS_AT)
#define MIPS_WSUBIUR(rd,is1,rs2)  MIPS_LA(MIPS_AT,is1), \
                                  MIPS_SUBU(rd,MIPS_AT,rs2)
#define MIPS_WTEQI(rs1,is2)       MIPS_LA(MIPS_AT,is2), \
                                  MIPS_TEQ(rs1,MIPS_AT)
#define MIPS_WTGEI(rs1,is2)       MIPS_LA(MIPS_AT,is2), \
                                  MIPS_TGE(rs1,MIPS_AT)
#define MIPS_WTGEIU(rs1,is2)      MIPS_LA(MIPS_AT,is2), \
                                  MIPS_TGEU(rs1,MIPS_AT)
#define MIPS_WTLTI(rs1,is2)       MIPS_LA(MIPS_AT,is2), \
                                  MIPS_TLT(rs1,MIPS_AT)
#define MIPS_WTLTIU(rs1,is2)      MIPS_LA(MIPS_AT,is2), \
                                  MIPS_TLTU(rs1,MIPS_AT)
#define MIPS_WTNEI(rs1,is2)       MIPS_LA(MIPS_AT,is2), \
                                  MIPS_TNE(rs1,MIPS_AT)
#define MIPS_WXORI(rd,rs1,is2)    MIPS_LA(MIPS_AT,is2), \
                                  MIPS_XOR(rd,rs1,MIPS_AT)

/**
 * instruction sizes
**/

/* loads and stores */
#define MIPS_LB_SIZE              0x04
#define MIPS_LBU_SIZE             0x04
#define MIPS_LD_SIZE              0x04
#define MIPS_LDL_SIZE             0x04
#define MIPS_LDR_SIZE             0x04
#define MIPS_LH_SIZE              0x04
#define MIPS_LHU_SIZE             0x04
#define MIPS_LL_SIZE              0x04
#define MIPS_LLD_SIZE             0x04
#define MIPS_LW_SIZE              0x04
#define MIPS_LWL_SIZE             0x04
#define MIPS_LWR_SIZE             0x04
#define MIPS_LWU_SIZE             0x04
#define MIPS_SB_SIZE              0x04
#define MIPS_SC_SIZE              0x04
#define MIPS_SCD_SIZE             0x04
#define MIPS_SD_SIZE              0x04
#define MIPS_SDL_SIZE             0x04
#define MIPS_SDR_SIZE             0x04
#define MIPS_SH_SIZE              0x04
#define MIPS_SW_SIZE              0x04
#define MIPS_SWL_SIZE             0x04
#define MIPS_SWR_SIZE             0x04
#define MIPS_SYNC_SIZE            0x04

/* integer operations */
#define MIPS_ADD_SIZE             0x04
#define MIPS_ADDI_SIZE            0x04
#define MIPS_ADDIU_SIZE           0x04
#define MIPS_ADDU_SIZE            0x04
#define MIPS_AND_SIZE             0x04
#define MIPS_ANDI_SIZE            0x04
#define MIPS_DADD_SIZE            0x04
#define MIPS_DADDI_SIZE           0x04
#define MIPS_DADDIU_SIZE          0x04
#define MIPS_DADDU_SIZE           0x04
#define MIPS_DDIV_SIZE            0x04
#define MIPS_DDIVU_SIZE           0x04
#define MIPS_DIV_SIZE             0x04
#define MIPS_DIVU_SIZE            0x04
#define MIPS_DMULT_SIZE           0x04
#define MIPS_DMULTU_SIZE          0x04
#define MIPS_DSLL_SIZE            0x04
#define MIPS_DSLL32_SIZE          0x04
#define MIPS_DSLLV_SIZE           0x04
#define MIPS_DSRA_SIZE            0x04
#define MIPS_DSRA32_SIZE          0x04
#define MIPS_DSRAV_SIZE           0x04
#define MIPS_DSRL_SIZE            0x04
#define MIPS_DSRL32_SIZE          0x04
#define MIPS_DSRLV_SIZE           0x04
#define MIPS_DSUB_SIZE            0x04
#define MIPS_DSUBU_SIZE           0x04
#define MIPS_LUI_SIZE             0x04
#define MIPS_MFHI_SIZE            0x04
#define MIPS_MFLO_SIZE            0x04
#define MIPS_MTHI_SIZE            0x04
#define MIPS_MTLO_SIZE            0x04
#define MIPS_MULT_SIZE            0x04
#define MIPS_MULTU_SIZE           0x04
#define MIPS_NOR_SIZE             0x04
#define MIPS_OR_SIZE              0x04
#define MIPS_ORI_SIZE             0x04
#define MIPS_SLL_SIZE             0x04
#define MIPS_SLLV_SIZE            0x04
#define MIPS_SLT_SIZE             0x04
#define MIPS_SLTI_SIZE            0x04
#define MIPS_SLTIU_SIZE           0x04
#define MIPS_SLTU_SIZE            0x04
#define MIPS_SRA_SIZE             0x04
#define MIPS_SRAV_SIZE            0x04
#define MIPS_SRL_SIZE             0x04
#define MIPS_SRLV_SIZE            0x04
#define MIPS_SUB_SIZE             0x04
#define MIPS_SUBU_SIZE            0x04
#define MIPS_XOR_SIZE             0x04
#define MIPS_XORI_SIZE            0x04

/* branching */
#define MIPS_BEQ_SIZE             0x04
#define MIPS_BEQL_SIZE            0x04
#define MIPS_BGEZ_SIZE            0x04
#define MIPS_BGEZAL_SIZE          0x04
#define MIPS_BGEZALL_SIZE         0x04
#define MIPS_BGEZL_SIZE           0x04
#define MIPS_BGTZ_SIZE            0x04
#define MIPS_BGTZL_SIZE           0x04
#define MIPS_BLEZ_SIZE            0x04
#define MIPS_BLEZL_SIZE           0x04
#define MIPS_BLTZ_SIZE            0x04
#define MIPS_BLTZAL_SIZE          0x04
#define MIPS_BLTZALL_SIZE         0x04
#define MIPS_BLTZL_SIZE           0x04
#define MIPS_BNE_SIZE             0x04
#define MIPS_BNEL_SIZE            0x04
#define MIPS_J_SIZE               0x04
#define MIPS_JAL_SIZE             0x04
#define MIPS_JALR_SIZE            0x04
#define MIPS_JR_SIZE              0x04

/* exceptions */
#define MIPS_BREAK_SIZE           0x04
#define MIPS_SYSCALL_SIZE         0x04
#define MIPS_TEQ_SIZE             0x04
#define MIPS_TEQI_SIZE            0x04
#define MIPS_TGE_SIZE             0x04
#define MIPS_TGEI_SIZE            0x04
#define MIPS_TGEIU_SIZE           0x04
#define MIPS_TGEU_SIZE            0x04
#define MIPS_TLT_SIZE             0x04
#define MIPS_TLTI_SIZE            0x04
#define MIPS_TLTIU_SIZE           0x04
#define MIPS_TLTU_SIZE            0x04
#define MIPS_TNE_SIZE             0x04
#define MIPS_TNEI_SIZE            0x04

/* system control */
#define MIPS_CACHE_SIZE           0x04
#define MIPS_ERET_SIZE            0x04
#define MIPS_MFC0_SIZE            0x04
#define MIPS_MTC0_SIZE            0x04
#define MIPS_TLBP_SIZE            0x04
#define MIPS_TLBR_SIZE            0x04
#define MIPS_TLBWI_SIZE           0x04
#define MIPS_TLBWR_SIZE           0x04

/* floating point operations */
#define MIPS_ABSD_SIZE            0x04
#define MIPS_ABSS_SIZE            0x04
#define MIPS_ADDD_SIZE            0x04
#define MIPS_ADDS_SIZE            0x04
#define MIPS_BC1F_SIZE            0x04
#define MIPS_BC1FL_SIZE           0x04
#define MIPS_BC1T_SIZE            0x04
#define MIPS_BC1TL_SIZE           0x04
#define MIPS_CEILLD_SIZE          0x04
#define MIPS_CEILLS_SIZE          0x04
#define MIPS_CEILWD_SIZE          0x04
#define MIPS_CEILWS_SIZE          0x04
#define MIPS_CEQD_SIZE            0x04
#define MIPS_CEQS_SIZE            0x04
#define MIPS_CFC1_SIZE            0x04
#define MIPS_CFD_SIZE             0x04
#define MIPS_CFS_SIZE             0x04
#define MIPS_CLED_SIZE            0x04
#define MIPS_CLES_SIZE            0x04
#define MIPS_CLTD_SIZE            0x04
#define MIPS_CLTS_SIZE            0x04
#define MIPS_CNGED_SIZE           0x04
#define MIPS_CNGES_SIZE           0x04
#define MIPS_CNGLD_SIZE           0x04
#define MIPS_CNGLED_SIZE          0x04
#define MIPS_CNGLES_SIZE          0x04
#define MIPS_CNGLS_SIZE           0x04
#define MIPS_CNGTD_SIZE           0x04
#define MIPS_CNGTS_SIZE           0x04
#define MIPS_COLED_SIZE           0x04
#define MIPS_COLES_SIZE           0x04
#define MIPS_COLTD_SIZE           0x04
#define MIPS_COLTS_SIZE           0x04
#define MIPS_CSEQD_SIZE           0x04
#define MIPS_CSEQS_SIZE           0x04
#define MIPS_CSFD_SIZE            0x04
#define MIPS_CSFS_SIZE            0x04
#define MIPS_CTC1_SIZE            0x04
#define MIPS_CUEQD_SIZE           0x04
#define MIPS_CUEQS_SIZE           0x04
#define MIPS_CULED_SIZE           0x04
#define MIPS_CULES_SIZE           0x04
#define MIPS_CULTD_SIZE           0x04
#define MIPS_CULTS_SIZE           0x04
#define MIPS_CUND_SIZE            0x04
#define MIPS_CUNS_SIZE            0x04
#define MIPS_CVTDL_SIZE           0x04
#define MIPS_CVTDS_SIZE           0x04
#define MIPS_CVTDW_SIZE           0x04
#define MIPS_CVTLD_SIZE           0x04
#define MIPS_CVTLS_SIZE           0x04
#define MIPS_CVTSD_SIZE           0x04
#define MIPS_CVTSL_SIZE           0x04
#define MIPS_CVTSW_SIZE           0x04
#define MIPS_CVTWD_SIZE           0x04
#define MIPS_CVTWS_SIZE           0x04
#define MIPS_DIVD_SIZE            0x04
#define MIPS_DIVS_SIZE            0x04
#define MIPS_DMFC1_SIZE           0x04
#define MIPS_DMTC1_SIZE           0x04
#define MIPS_FLOORLD_SIZE         0x04
#define MIPS_FLOORLS_SIZE         0x04
#define MIPS_FLOORWD_SIZE         0x04
#define MIPS_FLOORWS_SIZE         0x04
#define MIPS_LDC1_SIZE            0x04
#define MIPS_LWC1_SIZE            0x04
#define MIPS_MFC1_SIZE            0x04
#define MIPS_MTC1_SIZE            0x04
#define MIPS_MOVD_SIZE            0x04
#define MIPS_MOVS_SIZE            0x04
#define MIPS_MULD_SIZE            0x04
#define MIPS_MULS_SIZE            0x04
#define MIPS_NEGD_SIZE            0x04
#define MIPS_NEGS_SIZE            0x04
#define MIPS_ROUNDLD_SIZE         0x04
#define MIPS_ROUNDLS_SIZE         0x04
#define MIPS_ROUNDWD_SIZE         0x04
#define MIPS_ROUNDWS_SIZE         0x04
#define MIPS_SDC1_SIZE            0x04
#define MIPS_SQRTD_SIZE           0x04
#define MIPS_SQRTS_SIZE           0x04
#define MIPS_SUBD_SIZE            0x04
#define MIPS_SUBS_SIZE            0x04
#define MIPS_SWC1_SIZE            0x04
#define MIPS_TRUNCLD_SIZE         0x04
#define MIPS_TRUNCLS_SIZE         0x04
#define MIPS_TRUNCWD_SIZE         0x04
#define MIPS_TRUNCWS_SIZE         0x04

/* pseudo-instructions */
#define MIPS_ABS_SIZE             0x0C
#define MIPS_B_SIZE               0x08
#define MIPS_BAL_SIZE             0x08
#define MIPS_BEQZ_SIZE            0x08
#define MIPS_BEQZL_SIZE           0x08
#define MIPS_BGEI_SIZE            (MIPS_LA_SIZE+MIPS_BGER_SIZE)
#define MIPS_BGELI_SIZE           (MIPS_LA_SIZE+MIPS_BGELR_SIZE)
#define MIPS_BGELR_SIZE           0x0C
#define MIPS_BGELUI_SIZE          (MIPS_LA_SIZE+MIPS_BGELUR_SIZE)
#define MIPS_BGELUR_SIZE          0x0C
#define MIPS_BGER_SIZE            0x0C
#define MIPS_BGEUI_SIZE           (MIPS_LA_SIZE+MIPS_BGEUR_SIZE)
#define MIPS_BGEUR_SIZE           0x0C
#define MIPS_BGTI_SIZE            (MIPS_LA_SIZE+MIPS_BGTR_SIZE)
#define MIPS_BGTLI_SIZE           (MIPS_LA_SIZE+MIPS_BGTLR_SIZE)
#define MIPS_BGTLR_SIZE           0x0C
#define MIPS_BGTLUI_SIZE          (MIPS_LA_SIZE+MIPS_BGTLUR_SIZE)
#define MIPS_BGTLUR_SIZE          0x0C
#define MIPS_BGTR_SIZE            0x0C
#define MIPS_BGTUI_SIZE           (MIPS_LA_SIZE+MIPS_BGTUR_SIZE)
#define MIPS_BGTUR_SIZE           0x0C
#define MIPS_BLEI_SIZE            (MIPS_LA_SIZE+MIPS_BLER_SIZE)
#define MIPS_BLELI_SIZE           (MIPS_LA_SIZE+MIPS_BLELR_SIZE)
#define MIPS_BLELR_SIZE           0x0C
#define MIPS_BLELUI_SIZE          (MIPS_LA_SIZE+MIPS_BLELUR_SIZE)
#define MIPS_BLELUR_SIZE          0x0C
#define MIPS_BLER_SIZE            0x0C
#define MIPS_BLEUI_SIZE           (MIPS_LA_SIZE+MIPS_BLEUR_SIZE)
#define MIPS_BLEUR_SIZE           0x0C
#define MIPS_BLTI_SIZE            (MIPS_LA_SIZE+MIPS_BLTR_SIZE)
#define MIPS_BLTLI_SIZE           (MIPS_LA_SIZE+MIPS_BLTLR_SIZE)
#define MIPS_BLTLR_SIZE           0x0C
#define MIPS_BLTLUI_SIZE          (MIPS_LA_SIZE+MIPS_BLTLUR_SIZE)
#define MIPS_BLTLUR_SIZE          0x0C
#define MIPS_BLTR_SIZE            0x0C
#define MIPS_BLTUI_SIZE           (MIPS_LA_SIZE+MIPS_BLTUR_SIZE)
#define MIPS_BLTUR_SIZE           0x0C
#define MIPS_BNEZ_SIZE            0x08
#define MIPS_BNEZL_SIZE           0x08
#define MIPS_DIVR_SIZE            0x30
#define MIPS_DIVUI_SIZE           (MIPS_LA_SIZE+MIPS_DIVUR_SIZE)
#define MIPS_DIVUIR_SIZE          (MIPS_LA_SIZE+MIPS_DIVUR_SIZE)
#define MIPS_DIVUR_SIZE           0x18
#define MIPS_LA_SIZE              0x08
#define MIPS_LI_SIZE              0x04
#define MIPS_MOVE_SIZE            0x04
#define MIPS_MULI_SIZE            (MIPS_LA_SIZE+MIPS_MULR_SIZE)
#define MIPS_MULOI_SIZE           (MIPS_LA_SIZE+MIPS_MULOR_SIZE)
#define MIPS_MULOR_SIZE           0x28
#define MIPS_MULOUI_SIZE          (MIPS_LA_SIZE+MIPS_MULOUR_SIZE)
#define MIPS_MULOUR_SIZE          0x18
#define MIPS_MULR_SIZE            0x10
#define MIPS_NEG_SIZE             0x04
#define MIPS_NEGU_SIZE            0x04
#define MIPS_NOP_SIZE             0x04
#define MIPS_NOT_SIZE             0x04
#define MIPS_REMR_SIZE            0x30
#define MIPS_REMUI_SIZE           (MIPS_LA_SIZE+MIPS_REMUR_SIZE)
#define MIPS_REMUIR_SIZE          (MIPS_LA_SIZE+MIPS_REMUR_SIZE)
#define MIPS_REMUR_SIZE           0x18
#define MIPS_ROLI_SIZE            0x0C
#define MIPS_ROLR_SIZE            0x10
#define MIPS_RORI_SIZE            0x0C
#define MIPS_RORR_SIZE            0x10
#define MIPS_SEQI_SIZE            (MIPS_LA_SIZE+MIPS_SEQR_SIZE)
#define MIPS_SEQR_SIZE            0x08
#define MIPS_SGEI_SIZE            (MIPS_LA_SIZE+MIPS_SGER_SIZE)
#define MIPS_SGER_SIZE            0x08
#define MIPS_SGEUI_SIZE           (MIPS_LA_SIZE+MIPS_SGEUR_SIZE)
#define MIPS_SGEUR_SIZE           0x08
#define MIPS_SGTI_SIZE            (MIPS_LA_SIZE+MIPS_SGTR_SIZE)
#define MIPS_SGTR_SIZE            0x04
#define MIPS_SGTUI_SIZE           (MIPS_LA_SIZE+MIPS_SGTUR_SIZE)
#define MIPS_SGTUR_SIZE           0x04
#define MIPS_SLEI_SIZE            (MIPS_LA_SIZE+MIPS_SLER_SIZE)
#define MIPS_SLER_SIZE            0x08
#define MIPS_SLEUI_SIZE           (MIPS_LA_SIZE+MIPS_SLEUR_SIZE)
#define MIPS_SLEUR_SIZE           0x08
#define MIPS_SNEI_SIZE            (MIPS_LA_SIZE+MIPS_SNER_SIZE)
#define MIPS_SNER_SIZE            0x08
#define MIPS_SUBI_SIZE            0x04
#define MIPS_SUBIR_SIZE           0x08
#define MIPS_SUBIU_SIZE           0x04
#define MIPS_SUBIUR_SIZE          0x08
#define MIPS_ULH_SIZE             0x10
#define MIPS_ULHU_SIZE            0x10
#define MIPS_ULW_SIZE             0x08
#define MIPS_USH_SIZE             0x0C
#define MIPS_USW_SIZE             0x08
#define MIPS_WADDI_SIZE           (MIPS_LA_SIZE+0x04)
#define MIPS_WADDIU_SIZE          (MIPS_LA_SIZE+0x04)
#define MIPS_WANDI_SIZE           (MIPS_LA_SIZE+0x04)
#define MIPS_WORI_SIZE            (MIPS_LA_SIZE+0x04)
#define MIPS_WSUBI_SIZE           (MIPS_LA_SIZE+0x04)
#define MIPS_WSUBIR_SIZE          (MIPS_LA_SIZE+0x04)
#define MIPS_WSUBIU_SIZE          (MIPS_LA_SIZE+0x04)
#define MIPS_WSUBIUR_SIZE         (MIPS_LA_SIZE+0x04)
#define MIPS_WTEQI_SIZE           (MIPS_LA_SIZE+0x04)
#define MIPS_WTGEI_SIZE           (MIPS_LA_SIZE+0x04)
#define MIPS_WTGEIU_SIZE          (MIPS_LA_SIZE+0x04)
#define MIPS_WTLTI_SIZE           (MIPS_LA_SIZE+0x04)
#define MIPS_WTLTIU_SIZE          (MIPS_LA_SIZE+0x04)
#define MIPS_WTNEI_SIZE           (MIPS_LA_SIZE+0x04)
#define MIPS_WXORI_SIZE           (MIPS_LA_SIZE+0x04)

/**
 * concise names
**/

#ifdef MIPS_ASM_CONCISE

/* general purpose registers */
#define R0                        MIPS_R0
#define AT                        MIPS_AT
#define V0                        MIPS_V0
#define V1                        MIPS_V1
#define A0                        MIPS_A0
#define A1                        MIPS_A1
#define A2                        MIPS_A2
#define A3                        MIPS_A3
#define T0                        MIPS_T0
#define T1                        MIPS_T1
#define T2                        MIPS_T2
#define T3                        MIPS_T3
#define T4                        MIPS_T4
#define T5                        MIPS_T5
#define T6                        MIPS_T6
#define T7                        MIPS_T7
#define S0                        MIPS_S0
#define S1                        MIPS_S1
#define S2                        MIPS_S2
#define S3                        MIPS_S3
#define S4                        MIPS_S4
#define S5                        MIPS_S5
#define S6                        MIPS_S6
#define S7                        MIPS_S7
#define T8                        MIPS_T8
#define T9                        MIPS_T9
#define K0                        MIPS_K0
#define K1                        MIPS_K1
#define GP                        MIPS_GP
#define SP                        MIPS_SP
#define FP                        MIPS_FP
#define RA                        MIPS_RA

/* floating point registers */
#define F0                        MIPS_F0
#define F1                        MIPS_F1
#define F2                        MIPS_F2
#define F3                        MIPS_F3
#define F4                        MIPS_F4
#define F5                        MIPS_F5
#define F6                        MIPS_F6
#define F7                        MIPS_F7
#define F8                        MIPS_F8
#define F9                        MIPS_F9
#define F10                       MIPS_F10
#define F11                       MIPS_F11
#define F12                       MIPS_F12
#define F13                       MIPS_F13
#define F14                       MIPS_F14
#define F15                       MIPS_F15
#define F16                       MIPS_F16
#define F17                       MIPS_F17
#define F18                       MIPS_F18
#define F19                       MIPS_F19
#define F20                       MIPS_F20
#define F21                       MIPS_F21
#define F22                       MIPS_F22
#define F23                       MIPS_F23
#define F24                       MIPS_F24
#define F25                       MIPS_F25
#define F26                       MIPS_F26
#define F27                       MIPS_F27
#define F28                       MIPS_F28
#define F29                       MIPS_F29
#define F30                       MIPS_F30
#define F31                       MIPS_F31

/* loads and stores */
#define LB                        MIPS_LB
#define LBU                       MIPS_LBU
#define LD                        MIPS_LD
#define LDL                       MIPS_LDL
#define LDR                       MIPS_LDR
#define LH                        MIPS_LH
#define LHU                       MIPS_LHU
#define LL                        MIPS_LL
#define LLD                       MIPS_LLD
#define LW                        MIPS_LW
#define LWL                       MIPS_LWL
#define LWR                       MIPS_LWR
#define LWU                       MIPS_LWU
#define SB                        MIPS_SB
#define SC                        MIPS_SC
#define SCD                       MIPS_SCD
#define SD                        MIPS_SD
#define SDL                       MIPS_SDL
#define SDR                       MIPS_SDR
#define SH                        MIPS_SH
#define SW                        MIPS_SW
#define SWL                       MIPS_SWL
#define SWR                       MIPS_SWR
#define SYNC                      MIPS_SYNC

/* integer operations */
#define ADD                       MIPS_ADD
#define ADDI                      MIPS_ADDI
#define ADDIU                     MIPS_ADDIU
#define ADDU                      MIPS_ADDU
#define AND                       MIPS_AND
#define ANDI                      MIPS_ANDI
#define DADD                      MIPS_DADD
#define DADDI                     MIPS_DADDI
#define DADDIU                    MIPS_DADDIU
#define DADDU                     MIPS_DADDU
#define DDIV                      MIPS_DDIV
#define DDIVU                     MIPS_DDIVU
#define DIV                       MIPS_DIV
#define DIVU                      MIPS_DIVU
#define DMULT                     MIPS_DMULT
#define DMULTU                    MIPS_DMULTU
#define DSLL                      MIPS_DSLL
#define DSLL32                    MIPS_DSLL32
#define DSLLV                     MIPS_DSLLV
#define DSRA                      MIPS_DSRA
#define DSRA32                    MIPS_DSRA32
#define DSRAV                     MIPS_DSRAV
#define DSRL                      MIPS_DSRL
#define DSRL32                    MIPS_DSRL32
#define DSRLV                     MIPS_DSRLV
#define DSUB                      MIPS_DSUB
#define DSUBU                     MIPS_DSUBU
#define LUI                       MIPS_LUI
#define MFHI                      MIPS_MFHI
#define MFLO                      MIPS_MFLO
#define MTHI                      MIPS_MTHI
#define MTLO                      MIPS_MTLO
#define MULT                      MIPS_MULT
#define MULTU                     MIPS_MULTU
#define NOR                       MIPS_NOR
#define OR                        MIPS_OR
#define ORI                       MIPS_ORI
#define SLL                       MIPS_SLL
#define SLLV                      MIPS_SLLV
#define SLT                       MIPS_SLT
#define SLTI                      MIPS_SLTI
#define SLTIU                     MIPS_SLTIU
#define SLTU                      MIPS_SLTU
#define SRA                       MIPS_SRA
#define SRAV                      MIPS_SRAV
#define SRL                       MIPS_SRL
#define SRLV                      MIPS_SRLV
#define SUB                       MIPS_SUB
#define SUBU                      MIPS_SUBU
#define XOR                       MIPS_XOR
#define XORI                      MIPS_XORI

/* branching */
#define BEQ                       MIPS_BEQ
#define BEQL                      MIPS_BEQL
#define BGEZ                      MIPS_BGEZ
#define BGEZAL                    MIPS_BGEZAL
#define BGEZALL                   MIPS_BGEZALL
#define BGEZL                     MIPS_BGEZL
#define BGTZ                      MIPS_BGTZ
#define BGTZL                     MIPS_BGTZL
#define BLEZ                      MIPS_BLEZ
#define BLEZL                     MIPS_BLEZL
#define BLTZ                      MIPS_BLTZ
#define BLTZAL                    MIPS_BLTZAL
#define BLTZALL                   MIPS_BLTZALL
#define BLTZL                     MIPS_BLTZL
#define BNE                       MIPS_BNE
#define BNEL                      MIPS_BNEL
#define J                         MIPS_J
#define JAL                       MIPS_JAL
#define JALR                      MIPS_JALR
#define JR                        MIPS_JR

/* exceptions */
#define BREAK                     MIPS_BREAK
#define SYSCALL                   MIPS_SYSCALL
#define TEQ                       MIPS_TEQ
#define TEQI                      MIPS_TEQI
#define TGE                       MIPS_TGE
#define TGEI                      MIPS_TGEI
#define TGEIU                     MIPS_TGEIU
#define TGEU                      MIPS_TGEU
#define TLT                       MIPS_TLT
#define TLTI                      MIPS_TLTI
#define TLTIU                     MIPS_TLTIU
#define TLTU                      MIPS_TLTU
#define TNE                       MIPS_TNE
#define TNEI                      MIPS_TNEI

/* system control */
#define CACHE                     MIPS_CACHE
#define ERET                      MIPS_ERET
#define MFC0                      MIPS_MFC0
#define MTC0                      MIPS_MTC0
#define TLBP                      MIPS_TLBP
#define TLBR                      MIPS_TLBR
#define TLBWI                     MIPS_TLBWI
#define TLBWR                     MIPS_TLBWR

/* floating point operations */
#define ABSD                      MIPS_ABSD
#define ABSS                      MIPS_ABSS
#define ADDD                      MIPS_ADDD
#define ADDS                      MIPS_ADDS
#define BC1F                      MIPS_BC1F
#define BC1FL                     MIPS_BC1FL
#define BC1T                      MIPS_BC1T
#define BC1TL                     MIPS_BC1TL
#define CEILLD                    MIPS_CEILLD
#define CEILLS                    MIPS_CEILLS
#define CEILWD                    MIPS_CEILWD
#define CEILWS                    MIPS_CEILWS
#define CEQD                      MIPS_CEQD
#define CEQS                      MIPS_CEQS
#define CFC1                      MIPS_CFC1
#define CFD                       MIPS_CFD
#define CFS                       MIPS_CFS
#define CLED                      MIPS_CLED
#define CLES                      MIPS_CLES
#define CLTD                      MIPS_CLTD
#define CLTS                      MIPS_CLTS
#define CNGED                     MIPS_CNGED
#define CNGES                     MIPS_CNGES
#define CNGLD                     MIPS_CNGLD
#define CNGLED                    MIPS_CNGLED
#define CNGLES                    MIPS_CNGLES
#define CNGLS                     MIPS_CNGLS
#define CNGTD                     MIPS_CNGTD
#define CNGTS                     MIPS_CNGTS
#define COLED                     MIPS_COLED
#define COLES                     MIPS_COLES
#define COLTD                     MIPS_COLTD
#define COLTS                     MIPS_COLTS
#define CSEQD                     MIPS_CSEQD
#define CSEQS                     MIPS_CSEQS
#define CSFD                      MIPS_CSFD
#define CSFS                      MIPS_CSFS
#define CTC1                      MIPS_CTC1
#define CUEQD                     MIPS_CUEQD
#define CUEQS                     MIPS_CUEQS
#define CULED                     MIPS_CULED
#define CULES                     MIPS_CULES
#define CULTD                     MIPS_CULTD
#define CULTS                     MIPS_CULTS
#define CUND                      MIPS_CUND
#define CUNS                      MIPS_CUNS
#define CVTDL                     MIPS_CVTDL
#define CVTDS                     MIPS_CVTDS
#define CVTDW                     MIPS_CVTDW
#define CVTLD                     MIPS_CVTLD
#define CVTLS                     MIPS_CVTLS
#define CVTSD                     MIPS_CVTSD
#define CVTSL                     MIPS_CVTSL
#define CVTSW                     MIPS_CVTSW
#define CVTWD                     MIPS_CVTWD
#define CVTWS                     MIPS_CVTWS
#define DIVD                      MIPS_DIVD
#define DIVS                      MIPS_DIVS
#define DMFC1                     MIPS_DMFC1
#define DMTC1                     MIPS_DMTC1
#define FLOORLD                   MIPS_FLOORLD
#define FLOORLS                   MIPS_FLOORLS
#define FLOORWD                   MIPS_FLOORWD
#define FLOORWS                   MIPS_FLOORWS
#define LDC1                      MIPS_LDC1
#define LWC1                      MIPS_LWC1
#define MFC1                      MIPS_MFC1
#define MTC1                      MIPS_MTC1
#define MOVD                      MIPS_MOVD
#define MOVS                      MIPS_MOVS
#define MULD                      MIPS_MULD
#define MULS                      MIPS_MULS
#define NEGD                      MIPS_NEGD
#define NEGS                      MIPS_NEGS
#define ROUNDLD                   MIPS_ROUNDLD
#define ROUNDLS                   MIPS_ROUNDLS
#define ROUNDWD                   MIPS_ROUNDWD
#define ROUNDWS                   MIPS_ROUNDWS
#define SDC1                      MIPS_SDC1
#define SQRTD                     MIPS_SQRTD
#define SQRTS                     MIPS_SQRTS
#define SUBD                      MIPS_SUBD
#define SUBS                      MIPS_SUBS
#define SWC1                      MIPS_SWC1
#define TRUNCLD                   MIPS_TRUNCLD
#define TRUNCLS                   MIPS_TRUNCLS
#define TRUNCWD                   MIPS_TRUNCWD
#define TRUNCWS                   MIPS_TRUNCWS

/* pseudo-instructions */
#define ABS                       MIPS_ABS
#define B                         MIPS_B
#define BAL                       MIPS_BAL
#define BEQZ                      MIPS_BEQZ
#define BEQZL                     MIPS_BEQZL
#define BGEI                      MIPS_BGEI
#define BGELI                     MIPS_BGELI
#define BGELR                     MIPS_BGELR
#define BGELUI                    MIPS_BGELUI
#define BGELUR                    MIPS_BGELUR
#define BGER                      MIPS_BGER
#define BGEUI                     MIPS_BGEUI
#define BGEUR                     MIPS_BGEUR
#define BGTI                      MIPS_BGTI
#define BGTLI                     MIPS_BGTLI
#define BGTLR                     MIPS_BGTLR
#define BGTLUI                    MIPS_BGTLUI
#define BGTLUR                    MIPS_BGTLUR
#define BGTR                      MIPS_BGTR
#define BGTUI                     MIPS_BGTUI
#define BGTUR                     MIPS_BGTUR
#define BLEI                      MIPS_BLEI
#define BLELI                     MIPS_BLELI
#define BLELR                     MIPS_BLELR
#define BLELUI                    MIPS_BLELUI
#define BLELUR                    MIPS_BLELUR
#define BLER                      MIPS_BLER
#define BLEUI                     MIPS_BLEUI
#define BLEUR                     MIPS_BLEUR
#define BLTI                      MIPS_BLTI
#define BLTLI                     MIPS_BLTLI
#define BLTLR                     MIPS_BLTLR
#define BLTLUI                    MIPS_BLTLUI
#define BLTLUR                    MIPS_BLTLUR
#define BLTR                      MIPS_BLTR
#define BLTUI                     MIPS_BLTUI
#define BLTUR                     MIPS_BLTUR
#define BNEZ                      MIPS_BNEZ
#define BNEZL                     MIPS_BNEZL
#define DIVR                      MIPS_DIVR
#define DIVUI                     MIPS_DIVUI
#define DIVUIR                    MIPS_DIVUIR
#define DIVUR                     MIPS_DIVUR
#define LA                        MIPS_LA
#define LI                        MIPS_LI
#define MOVE                      MIPS_MOVE
#define MULI                      MIPS_MULI
#define MULOI                     MIPS_MULOI
#define MULOR                     MIPS_MULOR
#define MULOUI                    MIPS_MULOUI
#define MULOUR                    MIPS_MULOUR
#define MULR                      MIPS_MULR
#define NEG                       MIPS_NEG
#define NEGU                      MIPS_NEGU
#define NOP                       MIPS_NOP
#define NOT                       MIPS_NOT
#define REMR                      MIPS_REMR
#define REMUI                     MIPS_REMUI
#define REMUIR                    MIPS_REMUIR
#define REMUR                     MIPS_REMUR
#define ROLI                      MIPS_ROLI
#define ROLR                      MIPS_ROLR
#define RORI                      MIPS_RORI
#define RORR                      MIPS_RORR
#define SEQI                      MIPS_SEQI
#define SEQR                      MIPS_SEQR
#define SGEI                      MIPS_SGEI
#define SGER                      MIPS_SGER
#define SGEUI                     MIPS_SGEUI
#define SGEUR                     MIPS_SGEUR
#define SGTI                      MIPS_SGTI
#define SGTR                      MIPS_SGTR
#define SGTUI                     MIPS_SGTUI
#define SGTUR                     MIPS_SGTUR
#define SLEI                      MIPS_SLEI
#define SLER                      MIPS_SLER
#define SLEUI                     MIPS_SLEUI
#define SLEUR                     MIPS_SLEUR
#define SNEI                      MIPS_SNEI
#define SNER                      MIPS_SNER
#define SUBI                      MIPS_SUBI
#define SUBIR                     MIPS_SUBIR
#define SUBIU                     MIPS_SUBIU
#define SUBIUR                    MIPS_SUBIUR
#define ULH                       MIPS_ULH
#define ULHU                      MIPS_ULHU
#define ULW                       MIPS_ULW
#define USH                       MIPS_USH
#define USW                       MIPS_USW
#define WADDI                     MIPS_WADDI
#define WADDIU                    MIPS_WADDIU
#define WANDI                     MIPS_WANDI
#define WORI                      MIPS_WORI
#define WSUBI                     MIPS_WSUBI
#define WSUBIR                    MIPS_WSUBIR
#define WSUBIU                    MIPS_WSUBIU
#define WSUBIUR                   MIPS_WSUBIUR
#define WTEQI                     MIPS_WTEQI
#define WTGEI                     MIPS_WTGEI
#define WTGEIU                    MIPS_WTGEIU
#define WTLTI                     MIPS_WTLTI
#define WTLTIU                    MIPS_WTLTIU
#define WTNEI                     MIPS_WTNEI
#define WXORI                     MIPS_WXORI

#endif

/**
 * byte order utilities
**/

#ifndef MIPS_BYTE_ORDER_UNSUPPORTED
#if BYTE_ORDER == LITTLE_ENDIAN
#define mips_htom16               mips_swap16
#define mips_htom32               mips_swap32
#define mips_htom64               mips_swap64
#define mips_mtoh16               mips_swap16
#define mips_mtoh32               mips_swap32
#define mips_mtoh64               mips_swap64
#define mips_htom16b              mips_swap16b
#define mips_htom32b              mips_swap32b
#define mips_htom64b              mips_swap64b
#define mips_mtoh16b              mips_swap16b
#define mips_mtoh32b              mips_swap32b
#define mips_mtoh64b              mips_swap64b
#elif BYTE_ORDER == BIG_ENDIAN
#define mips_htom16               mips_nswap16_
#define mips_htom32               mips_nswap32_
#define mips_htom64               mips_nswap64_
#define mips_mtoh16               mips_nswap16_
#define mips_mtoh32               mips_nswap32_
#define mips_mtoh64               mips_nswap64_
#define mips_htom16b              mips_nswapb_
#define mips_htom32b              mips_nswapb_
#define mips_htom64b              mips_nswapb_
#define mips_mtoh16b              mips_nswapb_
#define mips_mtoh32b              mips_nswapb_
#define mips_mtoh64b              mips_nswapb_
#endif
#else
#define mips_swap_unsupported     _Static_assert(0, "byte order unsupported")
#define mips_htom16(...)          mips_swap_unsupported
#define mips_htom32(...)          mips_swap_unsupported
#define mips_htom64(...)          mips_swap_unsupported
#define mips_mtoh16(...)          mips_swap_unsupported
#define mips_mtoh32(...)          mips_swap_unsupported
#define mips_mtoh64(...)          mips_swap_unsupported
#define mips_htom16b(...)         mips_swap_unsupported
#define mips_htom32b(...)         mips_swap_unsupported
#define mips_htom64b(...)         mips_swap_unsupported
#define mips_mtoh16b(...)         mips_swap_unsupported
#define mips_mtoh32b(...)         mips_swap_unsupported
#define mips_mtoh64b(...)         mips_swap_unsupported
#endif

static inline uint16_t mips_nswap16_(uint16_t data)
{
  return data;
}

static inline uint32_t mips_nswap32_(uint32_t data)
{
  return data;
}

static inline uint64_t mips_nswap64_(uint64_t data)
{
  return data;
}

static inline uint16_t mips_swap16(uint16_t data)
{
  return ((data & 0x00FF) << 8) |
         ((data & 0xFF00) >> 8);
}

static inline uint32_t mips_swap32(uint32_t data)
{
  return ((data & 0x000000FF) << 24) |
         ((data & 0x0000FF00) << 8)  |
         ((data & 0x00FF0000) >> 8)  |
         ((data & 0xFF000000) >> 24);
}

static inline uint64_t mips_swap64(uint64_t data)
{
  return ((data & 0x00000000000000FF) << 56) |
         ((data & 0x000000000000FF00) << 40) |
         ((data & 0x0000000000FF0000) << 24) |
         ((data & 0x00000000FF000000) << 8)  |
         ((data & 0x000000FF00000000) >> 8)  |
         ((data & 0x0000FF0000000000) >> 24) |
         ((data & 0x00FF000000000000) >> 40) |
         ((data & 0xFF00000000000000) >> 56);
}

static inline void mips_nswapb_(void *data, size_t data_size)
{
}

static inline void mips_swap16b(void *data, size_t data_size)
{
  uint16_t *data_u16 = (uint16_t*)data;
  data_size /= sizeof(*data_u16);
  while (data_size--) {
    *data_u16 = mips_swap16(*data_u16);
    ++data_u16;
  }
}

static inline void mips_swap32b(void *data, size_t data_size)
{
  uint32_t *data_u32 = (uint32_t*)data;
  data_size /= sizeof(*data_u32);
  while (data_size--) {
    *data_u32 = mips_swap32(*data_u32);
    ++data_u32;
  }
}

static inline void mips_swap64b(void *data, size_t data_size)
{
  uint64_t *data_u64 = (uint64_t*)data;
  data_size /= sizeof(*data_u64);
  while (data_size--) {
    *data_u64 = mips_swap64(*data_u64);
    ++data_u64;
  }
}

#endif
