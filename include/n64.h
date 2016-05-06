/* n64.h
 * -glank
 */

#ifndef N64_H
#define N64_H

#if defined(__LANGUAGE_C__)
#define ENTRY __attribute__((section(".start")))
#elif defined(__cplusplus)
#define ENTRY extern "C" __attribute__((section(".start")))
#endif

static inline void init_gp()
{
  __asm__ volatile("la $gp, _gp");
}

#endif
