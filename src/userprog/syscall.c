#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
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

  void *esp = f->esp; // store the stack pointer for ease

  /* retrieve the system call number
   * by casting esp with an int pointer
   * and then dereferencing it to get
   * the value residing there. then we
   * pass it to a switch statement to
   * decide which system call is being
   * executed.
   *
   * note that in order to get any
   * subsequent arguments passed with
   * the system call number, we will need
   * to add 4 to esp for each argument, and
   * then cast esp the pointer of the
   * required data type (because esp is a
   *  void pointer, so it can be made to
   * anything with a cast) for the argument
   * and finally derefereing it to get the value
  */
  switch ( *(int *)esp ) {
    case SYS_HALT:
    {
      /* calling shutdown_power_off
       * as can be seen in
       * threads/init.c for when the
       * -q flag is set
       */
      shutdown_power_off();

      // used for debugging
      NOT_REACHED();
    }

    case SYS_EXIT:
    {
      /* before exiting, if the exiting
       * thread is a user programme,
       * we need to retrieve the exit
       * code for esp and then set it
       * for the current thread.
       * */
#ifdef USERPROG
      int exit_code = *(int *)( esp + 4 );

      thread_current()->exit_code = exit_code;
#endif

      thread_exit();

      // used for debugging
      MOT_REACHED();
    }

    case SYS_EXEC:
    {
      /* retrieve the first argument which
       * is the commad line to run, and
       * simply pass it to process execute
       * which will take care of everything
       * else.
       *
       * note that the function will not return
       * to caller due to the infinite for loop
       * in process_wait() due to child processes
       * not being implemented
       */
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
      /* retrieve the name of the file to remove
       * from the system, and then we need to lock
       * the file system to make sure the file to
       * be removed is not used anywhere (if it is
       * a kernel panic will happen) and to block any
       * future access until the removal is complete
       * then release the file system lock.
       *
       * finally a status of success or fail will be returned
       * */
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
      struct file *file = filesys_open( file_name );
      lock_release( &file_lock );

      if ( file == NULL ) { f->eax = -1; break; }

      /* find and allocate a free page for the file descriptor */
      struct file_map *file_m = (struct file_map *)malloc( sizeof( struct file_map * ) );

      f->eax = ( file_m->fd = thread_current()->current_fd++ );

      file_m->file = file;

      /* push the file descriptor onto the list */
      list_push_back( &thread_current()->file_list, &file_m->elem );

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

      else { /* reading from a file */ // (cyoon47, 2017)
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
      free( file_map );

      break;
    }

    default:
      printf( "syscall will not be implemented" );
      break;
  }
}




/* references:
 *
 * cyoon47 (2017) pintos-project-2 (master). Available from: https://github.com/cyoon47/pintos-project-2 [Accessed 04 December 2022]
 *
 */