/* startup.c
 * -glank
 */

#ifdef __cplusplus
extern "C"
{
#endif

static int global_ctors_done = 0;
static int global_dtors_done = 0;

extern void (*__CTOR_LIST__[])();
extern void (*__CTOR_END__[])();
extern void (*__DTOR_LIST__[])();
extern void (*__DTOR_END__[])();

void do_global_ctors()
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

void do_global_dtors()
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
