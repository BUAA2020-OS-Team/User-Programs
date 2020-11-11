#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

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
  //first check if f->esp is a valid pointer)
  /* if (f->esp is a bad pointer)
    {
      exit(-1);
    }*/

  // cast f->esp into an int*, then dereference it for the SYS_CODE
  switch(*(int*)f->esp)
  {
    case SYS_HALT:
    {
      // Implement syscall HALT
      break;
    }
    case SYS_EXIT:
    {
      // Implement syscall EXIT
      int status = 0;   // 暂时没想明白status保存在哪，先做简单处理。
      printf ("%s: exit(%d)\n", thread_current()->name, status);
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
