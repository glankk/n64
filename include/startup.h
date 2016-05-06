/* startup.h
 * -glank
 */

#ifndef STARTUP_H
#define STARTUP_H

#ifdef __cplusplus
extern "C"
{
#endif

void do_global_ctors();
void do_global_dtors();

#ifdef __cplusplus
}
#endif

#endif
