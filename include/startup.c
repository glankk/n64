/* startup.c
 * -glank
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

static int global_ctors_done = 0;
static int global_dtors_done = 0;

extern void *__bss_start, *__bss_end;
extern void (*__CTOR_LIST__[])(void);
extern void (*__CTOR_END__[])(void);
extern void (*__DTOR_LIST__[])(void);
extern void (*__DTOR_END__[])(void);

void clear_bss(void)
{
  uint8_t *bss_start = (uint8_t*)&__bss_start;
  uint8_t *bss_end = (uint8_t*)&__bss_end;
  while (bss_start < bss_end)
    *bss_start++ = 0x00;
}

void do_global_ctors(void)
{
  if (!global_ctors_done) {
    global_ctors_done = 1;
    global_dtors_done = 0;
    for (void (**ctor)() = __CTOR_LIST__; ctor != __CTOR_END__; ++ctor) {
      if (*ctor)
        (*ctor)();
    }
  }
}

void do_global_dtors(void)
{
  if (global_ctors_done && !global_dtors_done) {
    global_ctors_done = 0;
    global_dtors_done = 1;
    for (void (**dtor)() = __DTOR_LIST__; dtor != __DTOR_END__; ++dtor) {
      if (*dtor)
        (*dtor)();
    }
  }
}

#ifdef __cplusplus
}
#endif
