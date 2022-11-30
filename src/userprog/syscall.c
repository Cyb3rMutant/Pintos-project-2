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
      struct thread *curr = thread_current();

      struct list_elem *e;
      /* find the process that has to exit amongst its siblings and set its exit code (Pintos-Prooject-2) */
      for ( e = list_begin( &curr->parent->child_list ); e != list_end( &curr->parent->child_list );
        e = list_next( e ) ) {

        struct child *cp = list_entry( e, struct child, elem );

        if ( cp->tid == curr->tid ) {
          cp->used = 1;
          cp->exit_code = exit_code;
        }
      }

      curr->exit_code = exit_code;

      if ( curr->parent->waitingon == curr->tid )sema_up( &curr->parent->child_sema ); //increase sema if the exiting thread was in use

      thread_exit();

      MOT_REACHED();
    }

    case SYS_EXEC:
    {
      f->eax = process_execute( *(char **)( esp + 4 ) );
      break;
    }

    case SYS_WAIT:
    {
      f->eax = process_wait( *(int *)( esp + 4 ) );
      break;
    }

    case SYS_CREATE:
    {
      printf( "SYS_CREATE not implemented yet\n" );
      break;
    }
    case SYS_REMOVE:
    {
      printf( "SYS_REMOVE not implemented yet\n" );
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
      struct file_descriptor *file_d = palloc_get_page( 0 );

      f->eax = ( file_d->fd = thread_current()->current_fd++ );

      file_d->file = opened_file;

      /* push the file descriptor onto the list */
      list_push_back( &thread_current()->file_list, &file_d->elem );

      break;
    }

    case SYS_FILESIZE:
    {
      printf( "SYS_FILESIZE not implemented yet\n" );
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
        struct file *file = get_file_descriptor( &thread_current()->file_list, fd )->file;
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
        struct file *file = get_file_descriptor( &thread_current()->file_list, fd )->file;
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
      printf( "SYS_SEEK not implemented yet\n" );
      break;
    }

    case SYS_TELL:
    {
      printf( "SYS_TELL not implemented yet\n" );
      break;
    }

    case SYS_CLOSE:
    {
      struct file_descriptor *close_file_descriptor = get_file_descriptor( &thread_current()->file_list, *(int *)( esp + 4 ) );
      if ( close_file_descriptor == NULL ) break;

      /* close the file */
      lock_acquire( &file_lock );
      file_close( close_file_descriptor->file );
      lock_release( &file_lock );

      /* remove the file descriptor from the list */
      list_remove( &close_file_descriptor->elem );

      /* free the page allocated for the closed file descriptor */
      palloc_free_page( close_file_descriptor );

      break;
    }

    default:
      printf( "syscall will not be implemented" );
      break;
  }
}
