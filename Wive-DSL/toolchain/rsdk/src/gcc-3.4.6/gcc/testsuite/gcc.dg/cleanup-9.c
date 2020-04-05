/* { dg-do run { target i?86-*-linux* x86_64-*-linux* ia64-*-linux* alpha*-*-linux* powerpc*-*-linux* s390*-*-linux* sparc*-*-linux* mips*-*-linux* } } */
/* { dg-options "-fexceptions -fnon-call-exceptions -O2" } */
/* Verify that cleanups work with exception handling through realtime
   signal frames.  */

#include <unwind.h>
#include <stdlib.h>
#include <signal.h>

static _Unwind_Reason_Code
force_unwind_stop (int version, _Unwind_Action actions,
		   _Unwind_Exception_Class exc_class,
		   struct _Unwind_Exception *exc_obj,
		   struct _Unwind_Context *context, void *stop_parameter)
{
  if (actions & _UA_END_OF_STACK)
    abort ();
  return _URC_NO_REASON;
}

static void
force_unwind ()
{
  struct _Unwind_Exception *exc = malloc (sizeof (*exc));
  exc->exception_class = 0;
  exc->exception_cleanup = 0;

#ifndef __USING_SJLJ_EXCEPTIONS__
  _Unwind_ForcedUnwind (exc, force_unwind_stop, 0);
#else
  _Unwind_SjLj_ForcedUnwind (exc, force_unwind_stop, 0);
#endif

  abort ();
}

int count;
char *null;

static void
counter (void *p __attribute__ ((unused)))
{
  ++count;
}

static void
handler (void *p __attribute__ ((unused)))
{
  if (count != 2)
    abort ();
  exit (0);
}

static int __attribute__ ((noinline)) fn5 ()
{
  char dummy __attribute__ ((cleanup (counter)));
  force_unwind ();
  return 0;
}

static void
fn4 (int sig, siginfo_t * info, void *ctx)
{
  char dummy __attribute__ ((cleanup (counter)));
  fn5 ();
  null = NULL;
}

static void
fn3 ()
{
  abort ();
}

static int __attribute__ ((noinline)) fn2 ()
{
  *null = 0;
  fn3 ();
  return 0;
}

static int __attribute__ ((noinline)) fn1 ()
{
  struct sigaction s;
  sigemptyset (&s.sa_mask);
  s.sa_sigaction = fn4;
  s.sa_flags = SA_ONESHOT | SA_SIGINFO;
  sigaction (SIGSEGV, &s, NULL);
  fn2 ();
  return 0;
}

static int __attribute__ ((noinline)) fn0 ()
{
  char dummy __attribute__ ((cleanup (handler)));
  fn1 ();
  null = 0;
  return 0;
}

int
main ()
{
  fn0 ();
  abort ();
}
