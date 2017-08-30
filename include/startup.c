/* startup.c
 * -glank
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NO_STARTUP_LISTS
static __attribute__((section(".ctor_i"), used)) int32_t ctor_i = -1;
static __attribute__((section(".ctor_n"), used)) int32_t ctor_n = 0;
static __attribute__((section(".dtor_i"), used)) int32_t dtor_i = -1;
static __attribute__((section(".dtor_n"), used)) int32_t dtor_n = 0;
#endif

void clear_bss(void)
{
  extern void *__bss_start, *__bss_end;
  uint8_t *bss_start = (uint8_t*)&__bss_start;
  uint8_t *bss_end = (uint8_t*)&__bss_end;
  while (bss_start < bss_end)
    *bss_start++ = 0x00;
}

void do_global_ctors(void)
{
  extern void (*__CTOR_LIST__[])(void);
  static int global_ctors_done;
  if (global_ctors_done)
    return;
  int32_t i = 0;
  while (__CTOR_LIST__[i + 1])
    ++i;
  while (i > 0)
    __CTOR_LIST__[i--]();
  global_ctors_done = 1;
}

void do_global_dtors(void)
{
  extern void (*__DTOR_LIST__[])(void);
  static int global_dtors_done;
  if (global_dtors_done)
    return;
  for (void (**dtor)(void) = &__DTOR_LIST__[1]; *dtor; ++dtor)
    (*dtor)();
  global_dtors_done = 1;
}

#ifdef __cplusplus
}
#endif
