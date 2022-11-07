#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

static void syscall_handler( struct intr_frame * );

void
syscall_init( void ) {
  intr_register_int( 0x30, 3, INTR_ON, syscall_handler, "syscall" );
}


static void
syscall_handler( struct intr_frame *f ) {

  void *esp = f->esp;

  int syscall_no = *(int *)esp;

  switch ( syscall_no ) {
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
    {
#ifdef USERPROG
      int status;
      status = *(int *)( esp + 4 );
      thread_current()->exit_code = status;
#endif
      thread_exit();
      break;
    }

    case SYS_EXEC:
      printf( "SYS_EXEC not implemented yet\n" );
      break;

    case SYS_WAIT:
      printf( "SYS_WAIT not implemented yet\n" );
      break;

    case SYS_CREATE:
      printf( "SYS_CREATE not implemented yet\n" );
      break;

    case SYS_REMOVE:
      printf( "SYS_REMOVE not implemented yet\n" );
      break;

    case SYS_OPEN:
      printf( "SYS_OPEN not implemented yet\n" );
      break;

    case SYS_FILESIZE:
      printf( "SYS_FILESIZE not implemented yet\n" );
      break;

    case SYS_READ:
      printf( "SYS_READ not implemented yet\n" );
      break;

    case SYS_WRITE:
    {
      int write_fd = *(int *)( esp + 4 );
      char *write_buffer = *(char **)( esp + 8 );
      unsigned write_size = *(unsigned *)( esp + 12 );

      putbuf( write_buffer, write_size );
      f->eax = write_size;
      break;
    }

    case SYS_SEEK:
      printf( "SYS_SEEK not implemented yet\n" );
      break;

    case SYS_TELL:
      printf( "SYS_TELL not implemented yet\n" );
      break;

    case SYS_CLOSE:
      printf( "SYS_CLOSE not implemented yet\n" );
      break;

    default:
      printf( "syscall will not be implemented" );
      break;
  }
}
