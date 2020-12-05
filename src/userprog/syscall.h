#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

static struct lock rox_lock;

void syscall_init (void);
void exit(int status);

#endif /* userprog/syscall.h */
