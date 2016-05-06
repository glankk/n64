/* z64.h
 * -glank
 */

#ifndef Z64_H
#define Z64_H
#include <stdint.h>

#define Z64_OOT10     0x00
#define Z64_OOTDEBUG  0x01

#ifndef Z64_VERSION
#define Z64_VERSION   Z64_OOT10
#endif

typedef union
{
  struct
  {
    float x;
    float y;
    float z;
  };
  struct
  {
    uint32_t rx;
    uint32_t ry;
    uint32_t rz;
  };
} z64_xyz_t;

typedef struct
{
  uint16_t x;
  uint16_t y;
  uint16_t z;
} z64_rot_t;

enum
{
  BUTTON_C_RIGHT = 0x0001,
  BUTTON_C_LEFT  = 0x0002,
  BUTTON_C_DOWN  = 0x0004,
  BUTTON_C_UP    = 0x0008,
  BUTTON_R       = 0x0010,
  BUTTON_L       = 0x0020,
  BUTTON_D_RIGHT = 0x0100,
  BUTTON_D_LEFT  = 0x0200,
  BUTTON_D_DOWN  = 0x0400,
  BUTTON_D_UP    = 0x0800,
  BUTTON_START   = 0x1000,
  BUTTON_Z       = 0x2000,
  BUTTON_B       = 0x4000,
  BUTTON_A       = 0x8000,
};

typedef union
{
  struct
  {
    unsigned a  : 1;
    unsigned b  : 1;
    unsigned z  : 1;
    unsigned s  : 1;
    unsigned du : 1;
    unsigned dd : 1;
    unsigned dl : 1;
    unsigned dr : 1;
    unsigned    : 2;
    unsigned l  : 1;
    unsigned r  : 1;
    unsigned cu : 1;
    unsigned cd : 1;
    unsigned cl : 1;
    unsigned cr : 1;
    signed   x  : 8;
    signed   y  : 8;
  };
  uint16_t pad;
  uint32_t data;
} z64_controller_t;

typedef void (*SetTextRGBAProc)  (void* DList, uint8_t r, uint8_t g, uint8_t b,
                                  uint8_t a);
typedef void (*SetTextXYProc)    (void* DList, uint16_t x, uint16_t y);
typedef void (*SetTextStringProc)(void* DList, const char* format, ...);

#if Z64_VERSION == Z64_OOT10
#define z64_controller_1   (*(z64_controller_t*) 0x801C84B4)
#define z64_rupees         (*(uint16_t*)         0x8011A604)
#define z64_link_pos       (*(z64_xyz_t*)        0x801DAA54)
#define z64_link_rot       (*(z64_rot_t*)        0x801DAAE4)

#define SetTextRGBA        ((SetTextRGBAProc)    0x800CBE58)
#define SetTextXY          ((SetTextXYProc)      0x800CBEC8)
#define SetTextString      ((SetTextStringProc)  0x800CC588)
#endif

#if Z64_VERSION == Z64_OOTDEBUG
#define z64_controller_1   (*(z64_controller_t*) 0x801665C0)
#define z64_interface_off  (*(uint8_t*)          0x8015F9BF)

#define SetTextRGBA        ((SetTextRGBAProc)    0x800FB3AC)
#define SetTextXY          ((SetTextXYProc)      0x800FB41C)
#define SetTextString      ((SetTextStringProc)  0x800FBCB4)
#endif

#endif
