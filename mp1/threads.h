#ifndef THREADS_H_
#define THREADS_H_

#include "user/setjmp.h"

struct thread {
  struct thread *next;
  struct thread *previous;
  void *stack;
  void *stack_p;
  jmp_buf env; // for thread function
  int env_is_set;
  jmp_buf handler_env; // for signal handler function
  int handler_env_is_set;
  void (*fp)(void *arg);
  void *arg;
  void (*sig_handler[2])(int); // sig_handler[0] is for signo = 0,
                               // sig_handler[1] is for signo = 1
  int signo;                   // -1: no signal comes,
                               // 0: receive a signal signo = 0,
                               // 1: receive a signal signo = 1
  int ID;
};

struct thread *thread_create(void (*f)(void *), void *arg);
void thread_add_runqueue(struct thread *t);
void thread_yield(void);
void dispatch_signal(void);
void dispatch(void);
void schedule(void);
void thread_exit(void);
void thread_start_threading(void);
void thread_register_handler(int signo, void (*handler)(int));
void thread_kill(struct thread *t, int signo);

#endif // THREADS_H_
