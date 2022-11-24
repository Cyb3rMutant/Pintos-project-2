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

  int syscall_no = *(int *)esp;

  switch ( syscall_no ) {
    case SYS_HALT:
      shutdown_power_off();

      NOT_REACHED();
      break;

    case SYS_EXIT:
    {
      int status = *(int *)( esp + 4 );
      struct list_elem *e;

      for ( e = list_begin( &thread_current()->parent->child_proc ); e != list_end( &thread_current()->parent->child_proc );
        e = list_next( e ) ) {
        struct child *f = list_entry( e, struct child, elem );
        if ( f->tid == thread_current()->tid ) {
          f->used = true;
          f->exit_code = status;
        }
      }

      thread_current()->exit_code = status;

      if ( thread_current()->parent->waitingon == thread_current()->tid )
        sema_up( &thread_current()->parent->child_lock );

      thread_exit();
      break;
    }

    case SYS_EXEC:
    {

      char *cmd_line = *(char **)( esp + 4 );
      f->eax = process_execute( cmd_line );
      break;
    }

    case SYS_WAIT:
    {
      printf( "\n\n\nWWWWWWWWAAAAAAAAAAAAAAAIIIIIIIIIIIIIIIIIITTTTTTTTTTTTTT\n\n\n" );
      int pid = *(int *)( esp + 4 );
      f->eax = process_wait( pid );
      break;
    }

    case SYS_CREATE:
      printf( "SYS_CREATE not implemented yet\n" );
      break;

    case SYS_REMOVE:
      printf( "SYS_REMOVE not implemented yet\n" );
      break;

    case SYS_OPEN:
    {
      char *file_name = *(char **)( esp + 4 );
      acquire_file_lock();
      struct file *file_ptr = filesys_open( file_name );
      release_file_lock();

      if ( file_ptr == NULL )
        f->eax = -1;
      else {
        struct file_map *fmap = palloc_get_page( 0 );
        fmap->fd = thread_current()->next_fd++;
        fmap->file = file_ptr;
        list_push_back( &thread_current()->file_list, &fmap->elem );
        f->eax = fmap->fd;
      }

      break;
    }

    case SYS_FILESIZE:
      printf( "SYS_FILESIZE not implemented yet\n" );
      break;

    case SYS_READ:
    {
      int read_fd = *(int *)( esp + 4 );
      char *read_buffer = *(char **)( esp + 8 );
      unsigned read_size = *(unsigned *)( esp + 12 );

      if ( read_fd == 0 ) // read from keyboard
      {
        int i;
        for ( i = 0; i < read_size; i++ ) {
          read_buffer[i] = input_getc();
        }
        f->eax = read_size;
      }
      else {
        struct file *read_file = get_file( &thread_current()->file_list, read_fd );
        if ( read_file == NULL ) {
          f->eax = -1;
          return;
        }
        else {
          acquire_file_lock();
          f->eax = file_read( read_file, read_buffer, read_size );
          release_file_lock();
        }
      }

      break;
    }

    case SYS_WRITE:
    {
      int write_fd = *(int *)( esp + 4 );
      char *write_buffer = *(char **)( esp + 8 );
      unsigned write_size = *(unsigned *)( esp + 12 );

      if ( write_fd == 1 ) // writing to stdout
      {
        putbuf( write_buffer, write_size );
        f->eax = write_size;
      }
      else {
        struct file *write_file = get_file( &thread_current()->file_list, write_fd );
        if ( write_file == NULL ) {
          f->eax = -1;
          return;
        }
        else {
          acquire_file_lock();
          f->eax = file_write( write_file, write_buffer, write_size );
          release_file_lock();
        }
      }
      break;
    }

    case SYS_SEEK:
      printf( "SYS_SEEK not implemented yet\n" );
      break;

    case SYS_TELL:
      printf( "SYS_TELL not implemented yet\n" );
      break;

    case SYS_CLOSE:
    {
      int close_fd = *(int *)( esp + 4 );

      struct file_map *close_file_map = get_file_map( &thread_current()->file_list, close_fd );
      if ( close_file_map == NULL ) {
        return;
      }
      else {
        acquire_file_lock();
        file_close( close_file_map->file );
        release_file_lock();
        list_remove( &close_file_map->elem );
        palloc_free_page( close_file_map );
      }
      break;
    }

    default:
      printf( "syscall will not be implemented" );
      break;
  }
}
