/* startup.h
 * -glank
 */

#ifndef STARTUP_H
#define STARTUP_H

#if defined(__LANGUAGE_C__)
#define ENTRY __attribute__((section(".start")))
#elif defined(__cplusplus)
#define ENTRY extern "C" __attribute__((section(".start")))
extern "C"
{
#endif

void clear_bss(void);
void do_global_ctors(void);
void do_global_dtors(void);

#ifdef __cplusplus
}
#endif

static inline void init_gp(void)
{
  __asm__ volatile("la $gp, _gp");
}

#endif
