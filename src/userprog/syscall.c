#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "threads/palloc.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "userprog/process.h"


static void syscall_handler( struct intr_frame * );

void
syscall_init( void ) {
  intr_register_int( 0x30, 3, INTR_ON, syscall_handler, "syscall" );
}


static void
syscall_handler( struct intr_frame *f ) {

  void *esp = f->esp;

  switch ( *(int *)esp ) {
    case SYS_HALT:
    {
      shutdown_power_off();

      NOT_REACHED();
    }

    case SYS_EXIT: // (Pintos-Project-2)
    {
      int exit_code = *(int *)( esp + 4 );

#ifdef USERPROG
      thread_current()->exit_code = exit_code;
#endif

      thread_exit();

      MOT_REACHED();
    }

    case SYS_EXEC:
    {
      char *file_name = *(char **)( esp + 4 );

      f->eax = process_execute( file_name );
      break;
    }

    case SYS_WAIT:
    {
      printf( "SYS_WAIT not yet implemented\n" );
      break;
    }

    case SYS_CREATE:
    {
      printf( "SYS_CREATE not yet implemented\n" );
      break;
    }
    case SYS_REMOVE:
    {
      char *file_name = *(char **)( esp + 4 );

      lock_acquire( &file_lock );
      f->eax = filesys_remove( file_name );
      lock_release( &file_lock );

      break;
    }

    case SYS_OPEN:
    {
      char *file_name = *(char **)( esp + 4 );

      /* open the file */
      lock_acquire( &file_lock );
      struct file *opened_file = filesys_open( file_name );
      lock_release( &file_lock );

      if ( opened_file == NULL ) { f->eax = -1; break; }

      /* find and allocate a free page for the file descriptor */
      struct file_map *file_d = palloc_get_page( 0 );

      f->eax = ( file_d->fd = thread_current()->current_fd++ );

      file_d->file = opened_file;

      /* push the file descriptor onto the list */
      list_push_back( &thread_current()->file_list, &file_d->elem );

      break;
    }

    case SYS_FILESIZE:
    {
      int fd = *(int *)( esp + 4 );

      struct file *file = get_file_map( fd )->file;
      if ( file == NULL ) { f->eax = -1; break; }

      lock_acquire( &file_lock );
      f->eax = file_length( file );
      lock_release( &file_lock );
      break;
    }

    case SYS_READ:
    {
      int fd = *(int *)( esp + 4 );
      char *buf = *(char **)( esp + 8 );
      unsigned size = *(unsigned *)( esp + 12 );

      if ( fd == 0 ) for ( int i = 0; i < size; i++ ) buf[i] = input_getc(); // STDIN_FILENO

      else { /* reading from a file */
        /* find the file to read from */
        struct file *file = get_file_map( fd )->file;
        if ( file == NULL ) { f->eax = -1; break; }

        lock_acquire( &file_lock );
        size = file_read( file, buf, size );
        lock_release( &file_lock );
      }

      f->eax = size;

      break;
    }

    case SYS_WRITE:
    {
      int fd = *(int *)( esp + 4 );
      char *buf = *(char **)( esp + 8 );
      unsigned size = *(unsigned *)( esp + 12 );

      if ( fd == 1 ) putbuf( buf, size ); // STDOUT_FILENO

      else { /* writing to a file */
        /* find the file to write onto */
        struct file *file = get_file_map( fd )->file;
        if ( file == NULL ) { f->eax = -1; break; }

        lock_acquire( &file_lock );
        size = file_write( file, buf, size );
        lock_release( &file_lock );
      }

      f->eax = size;

      break;
    }

    case SYS_SEEK:
    {
      printf( "SYS_SEEK not yet implemented\n" );
      break;
    }

    case SYS_TELL:
    {
      printf( "SYS_TELL not yet implemented\n" );
      break;
    }

    case SYS_CLOSE:
    {
      int fd = *(int *)( esp + 4 );

      struct file_map *file_map = get_file_map( fd );
      if ( file_map == NULL ) break;

      /* close the file */
      lock_acquire( &file_lock );
      file_close( file_map->file );
      lock_release( &file_lock );

      /* remove the file descriptor from the list */
      list_remove( &file_map->elem );

      /* free the page allocated for the closed file descriptor */
      palloc_free_page( file_map );

      break;
    }

    default:
      printf( "syscall will not be implemented" );
      break;
  }
}