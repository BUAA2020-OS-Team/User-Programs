#include "userprog/syscall.h"
#include <stdio.h>
#include "lib/kernel/stdio.h"
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "devices/shutdown.h"
#include "process.h"
#include "filesys/filesys.h"

typedef int pid_t;

// fd与文件对应
struct file_fd
{
  struct file *file;
  int fd;
  struct list_elem file_elem;
};

static void syscall_handler (struct intr_frame *);
static void halt(void);
static pid_t exec (char *cmd_line);
static bool create (char *file, int initial_size);
static bool remove (char *file);
static int open (char *file);
static int filesize (int fd);
static void exit(int status);
static int read (int fd, const void* buffer, unsigned size);
static int write(int fd, const void* buffer, unsigned size);
static unsigned tell (int fd);
static void seek (int fd, unsigned position);
static void close (int fd);

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

  /* 这里的判断应该是有问题的 */
  // if (is_user_vaddr (f->esp) && f->esp > 0x08048000)
  // {
  //   if (pagedir_get_page (thread_current()->pagedir, f->esp) != NULL)
  //   {
  //     exit (-1);
  //   }
  // }
  // else
  // {
  //   exit (-1);
  // }
  
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
      char *cmd_line = (char *)(*((int*)f->esp + 1));
      f->eax = exec (cmd_line);
      break;
    }
    case SYS_WAIT:
    {
      // Implement syscall WAIT
      pid_t pid = *((pid_t*)f->esp + 1);
      process_wait(pid);
      break;
    }
    case SYS_CREATE:
    {
      // Implement syscall CREATE
      char *file = (char *)(*((int*)f->esp + 1));
      unsigned initial_size = *((unsigned*)f->esp + 2);
      f->eax = create (file, initial_size);
      break;
    }
    case SYS_REMOVE:
    {
      // Implement syscall REMOVE
      char *file = (char *)(*((int*)f->esp + 1));
      f->eax = remove (file);
      break;
    }
    case SYS_OPEN:
    {
      // Implement syscall OPEN
      char *file = (char *)(*((int*)f->esp + 1));
      f->eax = open (file);
      break;
    }
    case SYS_FILESIZE:
    {
      // Implement syscall FILESIZE
      int fd = *((int*)f->esp + 1);
      f->eax = filesize (fd);
      break;
    }
    case SYS_READ:
    {
      // Implement syscall READ
      int fd = *((int*)f->esp + 1);
      void* buffer = (void*)(*((int*)f->esp + 2));
      unsigned size = *((unsigned*)f->esp + 3);
      f->eax = read(fd, buffer, size);
      break;
    }
    case SYS_WRITE:
    {
      // Implement syscall WRITE
      int fd = *((int*)f->esp + 1);
      void* buffer = (void*)(*((int*)f->esp + 2));
      unsigned size = *((unsigned*)f->esp + 3);
      f->eax = write(fd, buffer, size);
      break;
    }
    case SYS_SEEK:
    {
      // Implement syscall SEEK
      int fd = *((int*)f->esp + 1);
      unsigned position = *((unsigned*)f->esp + 2);
      seek(fd, position);
      break;
    }
    case SYS_TELL:
    {
      // Implement syscall TELL
      int fd = *((int*)f->esp + 1);
      f->eax = tell(fd);
      break;
    }
    case SYS_CLOSE:
    {
      // Implement syscall CLOSE
      int fd = *((int*)f->esp + 1);
      close(fd);
      break;
    }
  }
}

static void halt (void) 
{
  shutdown_power_off ();
}

static void exit (int status) 
{
  printf ("%s: exit(%d)\n", thread_current()->name, status);
  sema_up(&thread_current()->parent->some_semaphore);
  thread_exit();
}

static pid_t exec (char *cmd_line) 
{
  return process_execute (cmd_line);
}

static bool create (char *file, int initial_size) 
{
  return filesys_create(file, initial_size);
}

static bool remove (char *file) 
{
  return filesys_remove (file);
}

static int open (char *file) 
{
  struct file *f = filesys_open (file);
  if (f == NULL) 
  {
    return -1;
  }
  else
  {
    thread_current ()->fd ++;
    int fd = thread_current ()->fd;
    struct file_fd *file_fd;
    file_fd->file = f;
    file_fd->fd = fd;
    list_push_back (&thread_current ()->file_list, &file_fd->file_elem);
    return fd;
  }
}

static int filesize (int fd) 
{
  struct list *file_list = &thread_current ()->file_list;
  struct list_elem *e;
  for (e = list_begin (file_list); e != list_end (file_list);
       e = list_next (e))
    {
      struct file_fd *file_fd = list_entry (e, struct file_fd, file_elem);
      if (file_fd->fd == fd)
      {
        struct file *file = file_fd->file;
        return file_length (file);
      }
    }
    return 0;
}

static int read (int fd, const void* buffer, unsigned size)
{
  // Not implement.
  return -1;
}

static int write (int fd, const void* buffer, unsigned size) 
{
  if (fd == 1)
  {
    putbuf ((char*)buffer, size);
  }
  // Not implement.
  return size;
}

static void seek (int fd, unsigned position) 
{
  // Not implement.
  return ;
}

static unsigned tell (int fd)
{
  // Not implement.
  return 0;
}

static void close (int fd) 
{
  // Not implement.
  return ;
}