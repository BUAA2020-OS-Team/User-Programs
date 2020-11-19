#include "userprog/syscall.h"
#include <stdio.h>
#include "lib/kernel/stdio.h"
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "threads/init.h"

static void syscall_handler (struct intr_frame *);
static void halt(void);
static void exit(int status);
static int write(int fd, const void* buffer, unsigned size);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  // thread_exit ();
  // first check if f->esp is a valid pointer)
  if (is_user_vaddr (f->esp) && f->esp > 0x08048000)
  {
    if (pagedir_get_page (thread_current()->pagedir, f->esp) != NULL)
    {
      exit (-1);
    }
  }
  else
  {
    exit (-1);
  }
  
  // cast f->esp into an int*, then dereference it for the SYS_CODE
  switch(*(int*)f->esp)
  {
    case SYS_HALT:
    {
      // Implement syscall HALT
      halt ();
      break;
    }
    case SYS_EXIT:
    {
      // Implement syscall EXIT
      int status = *((int*)f->esp + 1);
      exit (status);
      break;
    }
    case SYS_EXEC:
    {
      // Implement syscall EXIT
      break;
    }
    case SYS_WAIT:
    {
      // Implement syscall WAIT
      break;
    }
    case SYS_CREATE:
    {
      // Implement syscall CREATE
      break;
    }
    case SYS_REMOVE:
    {
      // Implement syscall REMOVE
      break;
    }
    case SYS_OPEN:
    {
      // Implement syscall OPEN
      break;
    }
    case SYS_FILESIZE:
    {
      // Implement syscall FILESIZE
      break;
    }
    case SYS_READ:
    {
      // Implement syscall READ
      break;
    }
    case SYS_WRITE:
    {
      // Implement syscall WRITE
      int fd = *((int*)f->esp + 1);
      void* buffer = (void*)(*((int*)f->esp + 2));
      unsigned size = *((unsigned*)f->esp + 3);
      //run the syscall, a function of your own making
      //since this syscall returns a value, the return value should be stored in f->eax
      f->eax = write(fd, buffer, size);
      break;
    }
    case SYS_SEEK:
    {
      // Implement syscall SEEK
      break;
    }
    case SYS_TELL:
    {
      // Implement syscall TELL
      break;
    }
    case SYS_CLOSE:
    {
      // Implement syscall CLOSE
      break;
    }
  }
}

static void halt (void) {
  shutdown_power_off ();
}

static void exit (int status) {
  printf ("%s: exit(%d)\n", thread_current()->name, status);
  sema_up(&thread_current()->parent->some_semaphore);
  thread_exit();
}

static int write (int fd, const void* buffer, unsigned size) {
  if (fd == 1)
  {
    putbuf ((char*)buffer, size);
  }
}
