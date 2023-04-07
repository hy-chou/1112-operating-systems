#include "user/threads.h"
#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/user.h"

#define NULL 0
#define NULL_FUNC ((void (*)(int)) - 1)

static struct thread *current_thread = NULL;
static int id = 1;
static jmp_buf env_tmp;
static jmp_buf env_main;

struct thread *thread_create(void (*f)(void *), void *arg) {
  struct thread *t = (struct thread *)malloc(sizeof(struct thread));

  t->stack = malloc(sizeof(unsigned long) * 0x100);
  t->stack_p = t->stack + 0x100 * 8 - 0x2 * 8;
  t->env_is_set = 0;
  t->handler_env_is_set = 0;
  t->fp = f;
  t->arg = arg;
  t->sig_handler[0] = NULL_FUNC;
  t->sig_handler[1] = NULL_FUNC;
  t->signo = -1;
  t->ID = id;
  id++;

  return t;
}

void thread_add_runqueue(struct thread *t) {
  if (current_thread == NULL) {
    // TODO
    t->next = t;
    t->previous = t;
    current_thread = t;
  } else {
    // TODO
    t->next = current_thread;
    t->previous = current_thread->previous;
    current_thread->previous->next = t;
    current_thread->previous = t;

    t->sig_handler[0] = current_thread->sig_handler[0];
    t->sig_handler[1] = current_thread->sig_handler[1];
  }
}

void thread_yield(void) {
  // TODO
  if (current_thread->signo != -1) {                // called by signal handler
    if (setjmp(current_thread->handler_env) == 0) { // after setjmp
      current_thread->handler_env_is_set = 1;
      schedule();
      dispatch();
    } else { // after longjmp
      // do nothing and return to the calling place
    }
  } else {                                  // called by thread function
    if (setjmp(current_thread->env) == 0) { // after setjmp
      current_thread->env_is_set = 1;
      schedule();
      dispatch();
    } else { // after longjmp
      // do nothing and return to the calling place
    }
  }
}

void dispatch_signal(void) {
  if (current_thread->sig_handler[current_thread->signo] == NULL_FUNC) {
    thread_exit();
  }

  if (current_thread->handler_env_is_set) {
    longjmp(current_thread->handler_env, 1);
  }

  if (setjmp(env_tmp) == 0) {
    if (current_thread->env_is_set) {
      env_tmp->sp = current_thread->env->sp;
    } else {
      env_tmp->sp = (unsigned long)current_thread->stack_p;
    }
    longjmp(env_tmp, 1);
  }
  current_thread->sig_handler[current_thread->signo](current_thread->signo);
  current_thread->signo = -1;
  dispatch();
}

void dispatch(void) {
  // TODO
  if (current_thread->signo != -1) { // signo == 0 or 1
    dispatch_signal();
  } else { // signo == -1
    if (current_thread->env_is_set) {
      longjmp(current_thread->env, 1);
    }

    if (setjmp(env_tmp) == 0) {
      env_tmp->sp = (unsigned long)current_thread->stack_p;
      longjmp(env_tmp, 1);
    }
    current_thread->fp(current_thread->arg);
    thread_exit();
  }
}

void schedule(void) {
  // TODO
  current_thread = current_thread->next;
}

void thread_exit(void) {
  if (current_thread->next != current_thread) { // thread count >= 2
    // TODO
    struct thread *tmp_thread = current_thread->next;

    current_thread->next->previous = current_thread->previous;
    current_thread->previous->next = current_thread->next;

    free(current_thread->stack);
    free(current_thread);

    current_thread = tmp_thread;
    dispatch();
  } else { // thread count == 1
    // TODO
    free(current_thread->stack);
    free(current_thread);

    current_thread = NULL;
    // Hint: No more thread to execute
    longjmp(env_main, 1);
  }
}

void thread_start_threading(void) {
  // TODO
  if (setjmp(env_main) == 0) {
    dispatch();
  } else {
    // do nothing and return to main function
  }
}

void thread_register_handler(int signo, void (*handler)(int)) {
  // TODO
  current_thread->sig_handler[signo] = handler;
}

void thread_kill(struct thread *t, int signo) {
  // TODO
  t->signo = signo;
}
