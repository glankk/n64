/* icxxabi.cpp
 * -glank
 */

#include <cstdlib>
#include <cinttypes>
#define ATEXIT_MAX 128

using atexit_func = void (*)(void *arg);
struct atexit_entry
{
  atexit_func   func;
  void         *arg;
  void         *dso_handle;
};


void *operator new(size_t size)
{
  return malloc(size);
}
void *operator new[](size_t size)
{
  return malloc(size);
}

void operator delete(void *p) noexcept
{
  free(p);
}
void operator delete[](void *p) noexcept
{
  free(p);
}


extern "C"
{

void                   *__dso_handle = nullptr;
static atexit_entry_t   atexit_entries[ATEXIT_MAX] {};
static size_t           atexit_count = 0;

int __cxa_guard_acquire(int64_t *g)
{
  return !*g;
}

void __cxa_guard_release(int64_t *g)
{
  *g = 1;
}

void __cxa_guard_abort(int64_t *g)
{
}

void __cxa_pure_virtual()
{
}

int __cxa_atexit(atexit_func func, void *arg, void *dso_handle)
{
  if (atexit_count >= ATEXIT_MAX)
    return -1;
  atexit_entries[atexit_count++] = {func, arg, dso_handle};
  return 0;
}

void __cxa_finalize(atexit_func func)
{
  if (func) {
    for (size_t i = 0; i < atexit_count; ++i)
      if (atexit_entries[i].func == func) {
        atexit_entries[i].func(atexit_entries[i].arg);
        while (++i < atexit_count)
          atexit_entries[i - 1] = atexit_entries[i];
        --atexit_count;
        break;
      }
  }
  else {
    for (size_t i = atexit_count - 1; i >= 0; --i)
      atexit_entries[i].func(atexit_entries[i].arg);
    atexit_count = 0;
  }
}

}
