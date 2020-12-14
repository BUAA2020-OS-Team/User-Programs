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
#include "filesys/file.h"

typedef int pid_t;

// fd与文件对应
struct file_fd
{
  struct file *file;
  int fd;
  struct list_elem file_elem;
};

struct write_thread
{
    struct thread *t;
    struct file *file;
};

static struct write_thread *wt;

static void syscall_handler (struct intr_frame *);
static void halt(void);
static pid_t exec (char *cmd_line);
static int wait (pid_t pid);
static bool create (char *file, int initial_size);
static bool remove (char *file);
static int open (char *file);
static int filesize (int fd);
static int read (int fd, void* buffer, unsigned size);
static int write(int fd, void* buffer, unsigned size);
static unsigned tell (int fd);
static void seek (int fd, unsigned position);
static void close (int fd);
static int get_user (const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byte);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&rox_lock);
}

static bool isBad (const void *p)
{
  if (p <= (void*)0xbffffffc && p > (void*)0x08048000)
  {
    if (pagedir_get_page (thread_current()->pagedir, p) == NULL)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  // thread_exit ();
  // first check if f->esp is a valid pointer)

  // printf ("%%esp: %p, [%%esp]: %d", f->esp, *(int*)f->esp);
  
  /* 这里的判断应该是有问题的 */
  if (f->esp <= (void*)0xbffffffc && f->esp > (void*)0x08048000)
  {
    if (pagedir_get_page (thread_current()->pagedir, f->esp) == NULL)
    {
      exit (-1);
    }
  }
  else
  {
    exit (-1);
  }
  
  // printf ("%%esp: %p, [%%esp]: %d", f->esp, *(int*)f->esp);
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
      if (!is_user_vaddr ((int*)f->esp + 1))
      {
        exit (-1);
      }
      int status = *((int*)f->esp + 1);
      exit (status);
      break;
    }
    case SYS_EXEC:
    {
      // Implement syscall EXEC
      char *cmd_line = (char *)(*((int*)f->esp + 1));
      f->eax = exec (cmd_line);
      break;
    }
    case SYS_WAIT:
    {
      // Implement syscall WAIT
      pid_t pid = *((pid_t*)f->esp + 1);
      f->eax = wait(pid);
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
    default:
      exit (-1);
  }
}

static void halt (void) 
{
  shutdown_power_off ();
}

void exit (int status) 
{
  printf ("%s: exit(%d)\n", thread_current()->name, status);
  struct list_elem *e;
  struct thread *t = thread_current ();
  for (e = list_begin (&t->parent->ct_list); e != list_end (&t->parent->ct_list);
      e = list_next (e))
    {
      struct cthread *ct = list_entry (e, struct cthread, ctelem);
      if (ct->cthread->tid == t->tid) 
        {
          ct->exit_status = status;
          break;
        }     
    }
    
  if (thread_current()->parent->cur_waitpid == thread_current()->tid)
    sema_up(&thread_current()->parent->some_semaphore);

  thread_exit();
}

static pid_t exec (char *cmd_line) 
{
  if (cmd_line == NULL)
    exit (-1);
  if (isBad (cmd_line))
    exit (-1);

  // struct file *f = filesys_open (cmd_line);
  // if (f == NULL)
  //   return -1;
  // else
  //   file_close (f);

  lock_acquire (&rox_lock);
  pid_t pid = process_execute (cmd_line);
  lock_release (&rox_lock);
  return pid;
}

static int wait (pid_t pid) 
{
  if (thread_current()->cur_waitpid != 0)
    return -1;
  else 
    thread_current()->cur_waitpid = (tid_t)pid;

  //printf ("%d\n", pid);

  struct list_elem *e;
  bool isChild = false;

  for (e = list_begin (&thread_current()->ct_list); e != list_end (&thread_current()->ct_list);
      e = list_next (e))
    {
      struct cthread *ct = list_entry (e, struct cthread, ctelem);
      if (ct->cthread->tid == (tid_t)pid)
        isChild = true;
    }
  if (!isChild)
    return -1;

  // while (!sema_try_down(&thread_current()->some_semaphore)) 
  //     if (process_wait((tid_t)pid) == -1)
  //       break;
  thread_current()->cur_waitpid = 0;
  return process_wait((tid_t)pid);
}

static bool create (char *file, int initial_size) 
{
  if (file == NULL)
    exit (-1);
  if (isBad (file))
    exit (-1);
  return filesys_create(file, initial_size);
}

static bool remove (char *file) 
{
  return filesys_remove (file);
}

static int open (char *file) 
{
  if (file == NULL)
    return -1;
  if (isBad (file))
    exit (-1);
  lock_acquire (&rox_lock);
  struct file *f = filesys_open (file);
  lock_release (&rox_lock);
  if (f == NULL) 
  {
    return -1;
  }
  else
  {
    thread_current ()->fd = thread_current ()->fd + 1;
    int fd = thread_current ()->fd;
    struct file_fd *ff = malloc(sizeof(*ff));
    ff->file = f;
    ff->fd = fd;
    list_push_back (&thread_current ()->file_list, &ff->file_elem);
    wt = malloc(sizeof(wt));
    return fd;
  }
}

static int filesize (int fd) 
{
  if (fd > thread_current ()->fd)
    return 0;
  struct list_elem *e;
  for (e = list_begin (&thread_current ()->file_list); e != list_end (&thread_current ()->file_list);
       e = list_next (e))
    {
      struct file_fd *ff = list_entry (e, struct file_fd, file_elem);
      if (ff->fd == fd)
      {
        return file_length (ff->file);
      }
    }
    return 0;
}

static int read (int fd, void *buffer, unsigned size)
{
  
  if (fd == 1 || fd > thread_current ()->fd)
    return -1;
  if (isBad (buffer))
    exit (-1);
  int i = 0;
  if (fd == 0)
  {
    // Read from stdin
    char *buffer = buffer;
    while (i < size)
    {
      uint8_t key = input_getc();
      //memset ((char*)buffer, key, 1);
      buffer[i] = key;
      i++;
    }
    return size;
  }
  else
  {
    struct list_elem *e;
    struct file *f = NULL;
    for (e = list_begin (&thread_current ()->file_list); e != list_end (&thread_current ()->file_list);
       e = list_next (e))
    {
      struct file_fd *ff = list_entry (e, struct file_fd, file_elem);
      if (ff->fd == fd)
      {
        f = ff->file;
        break;
      }
    }
    if (f == NULL)
      return -1;
    lock_acquire (&rox_lock);
    int res = file_read (f, buffer, size);
    file_deny_write (f);
    lock_release (&rox_lock);
    return res;
  }
}

static int write (int fd, void* buffer, unsigned size) 
{
  if (fd == 1)
  {
    putbuf ((char*)buffer, size);
    return size;
  }
  if (fd == 0 || fd > thread_current ()->fd)
    return -1;
  if (isBad (buffer))
    exit (-1);
  else
  {
    struct list_elem *e;
    struct file *f = NULL;
    for (e = list_begin (&thread_current ()->file_list); e != list_end (&thread_current ()->file_list);
        e = list_next (e))
    {
      struct file_fd *ff = list_entry (e, struct file_fd, file_elem);
      if (ff->fd == fd)
      {
        f = ff->file;
        break;
      }
    }
    if (f == NULL)
      return -1;
    lock_acquire (&rox_lock);
    if (wt->file == f && wt->t == thread_current ())
      file_allow_write (f);
    int res = file_write (f, buffer, size);
    wt->file = f;
    wt->t = thread_current ();
    //printf ("%d", res);
    file_deny_write (f);
    lock_release (&rox_lock);
    return res;
  }
  
}

static void seek (int fd, unsigned position) 
{
  if (fd > thread_current ()->fd)
    return;
  struct list_elem *e;
  struct file *f = NULL;
  for (e = list_begin (&thread_current ()->file_list); e != list_end (&thread_current ()->file_list);
      e = list_next (e))
  {
    struct file_fd *ff = list_entry (e, struct file_fd, file_elem);
    if (ff->fd == fd)
    {
      f = ff->file;
      break;
    }
  }
  if (f == NULL)
    return;
  file_seek (f, position);
  file_allow_write (f);
  return ;
}

static unsigned tell (int fd)
{
  if (fd > thread_current ()->fd)
    return;
  struct list_elem *e;
  struct file *f = NULL;
  for (e = list_begin (&thread_current ()->file_list); e != list_end (&thread_current ()->file_list);
      e = list_next (e))
  {
    struct file_fd *ff = list_entry (e, struct file_fd, file_elem);
    if (ff->fd == fd)
    {
      f = ff->file;
      break;
    }
  }
  if (f == NULL)
    return 0;
  return file_tell (f);
}

static void close (int fd) 
{
  if (fd == 0 || fd == 1 || fd > thread_current ()->fd)
    return ;
  struct list_elem *e;
  struct file *f = NULL;
  struct file_fd *ff;
  for (e = list_begin (&thread_current ()->file_list); e != list_end (&thread_current ()->file_list);
      e = list_next (e))
  {
    ff = list_entry (e, struct file_fd, file_elem);
    if (ff->fd == fd)
    {
      f = ff->file;
      break;
    }
  }
  if (f == NULL)
    return ;
  list_remove (&ff->file_elem);
  file_allow_write (f);
  free (wt);
  file_close (f);
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}