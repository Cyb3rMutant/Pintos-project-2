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
      */
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
      f->eax = -1;
      break;
    }

    case SYS_CREATE:
    {
      printf( "SYS_CREATE not yet implemented\n" );
      f->eax = -1;
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
      */
      char *file_name = *(char **)( esp + 4 );

      lock_acquire( &file_lock );
      f->eax = filesys_remove( file_name );
      lock_release( &file_lock );

      break;
    }

    case SYS_OPEN:
    {
      /* retrieve the name of the file to open
       * from the system
      */
      char *file_name = *(char **)( esp + 4 );

      /* open the file after locking the file
       * system to block access to the file, to
       * avoid any alterations to it, for example
       * a change in its name. and once the opening
       * is complete we release the lock.
       * we store it a pre defined file structure
       * pointer to be later stored in the file map
       * structure we made in thread.h
      */
      lock_acquire( &file_lock );
      struct file *file = filesys_open( file_name );
      lock_release( &file_lock );

      /* make sure the file was found and opened,
       * otherwise return a status fail denoted by
       * -1 because no file descriptor can be below 0
      */
      if ( file == NULL ) { f->eax = -1; break; }

      /* create a file map and allocate a free
       * chunck of memory of the same size as
       * its pointer size. palloc can be used
       * here but because we know how much do
       * need malloc does a better job than
       * holding a full page's size for it
      */
      struct file_map *file_m = (struct file_map *)malloc( sizeof( struct file_map * ) );

      /* get the current available file
       * descriptor number and allocate
       * it to the file we opened and
       * increment the file descriptor
       * counter of the parent process
       * for any further files to be
       * opened. and then return the
       * file descriptor we got to eax
      */
      f->eax = ( file_m->fd = thread_current()->current_fd++ );

      /* store the file structure we
       * opened in the file map file
       * pointer
      */
      file_m->file = file;

      /* push the file map we created
       * that stores the file pointer,
       * and the file descriptor onto
       * the file list of its parent
       * process using the list elem
       * structure that is used to
       * keep track of linked lists
       */
      list_push_back( &thread_current()->file_list, &file_m->elem );

      break;
    }

    case SYS_FILESIZE:
    {
      /* retrieve the file descriptor
       * number of the opened file
       * that we want to check its size
      */
      int fd = *(int *)( esp + 4 );

      /* retrieve the file map from the
       * file list of the current running
       * process using its file descriptor
       * and then store its file pointer
      */
      struct file *file = get_file_map( fd )->file;

      /* make sure the file was found,
       * otherwise return a status fail
       * denoted by -1 because no file
       * size can be smaller than zero
      */
      if ( file == NULL ) { f->eax = -1; break; }

      /* lock the file system to make
       * sure the file we are checking
       * its size is not altered by
       * writing to it for example.
       * and then return the size to
       * eax and release the lock
      */
      lock_acquire( &file_lock );
      f->eax = file_length( file );
      lock_release( &file_lock );
      break;

    }

    case SYS_READ:
    {
      /* retrieve the file descriptor
       * of where from are we reading,
       * the buffer to read into and
       * the size of bytes to read
       * into the buffer form esp
       * */
      int fd = *(int *)( esp + 4 );
      char *buf = *(char **)( esp + 8 );
      unsigned size = *(unsigned *)( esp + 12 );

      /* if the file descriptor is 0
       * it means we are reading from the
       * terminal, so we loop size many
       * times and read character by
       * character from the terminal
       * into the buffer*
      */
      if ( fd == 0 ) for ( int i = 0; i < size; i++ ) buf[i] = input_getc(); // STDIN_FILENO

      /* ohterwise, if the file descriptor
       * is anything other than 1, which
       * means we are reading from an
       * opened file from the file system
      */
      else { /* reading from a file */

        /* retrieve the file map from the
         * file list of the current running
         * process using its file descriptor
         * and then store its file pointer
        */
        struct file *file = get_file_map( fd )->file;

        /* make sure the file was found,
         * otherwise return a status fail
         * denoted by -1 because we can't
         * read less than 0 characters
        */
        if ( file == NULL ) { f->eax = -1; break; }

        /* lock the file system to make
         * sure the file we are reading
         * from is not altered by
         * writing to it for example.
         * and then store the size
         * of bytes read to be returned
         * to eax because we could have
         * read less than the expected
         * size and release the lock
        */
        lock_acquire( &file_lock );
        size = file_read( file, buf, size );
        lock_release( &file_lock );
      }

      /* return the size of bytes
       * read from either the terminal
       * or a file from the file system
      */
      f->eax = size;

      break;
    }

    case SYS_WRITE:
    {
      /* retrieve the file descriptor
       * of where to are we writing,
       * the buffer to write from and
       * the size of bytes to write
       * from the buffer form esp
       * */
      int fd = *(int *)( esp + 4 );
      char *buf = *(char **)( esp + 8 );
      unsigned size = *(unsigned *)( esp + 12 );

      /* if the file descriptor is 1
       * it means we are writing to the
       * terminal, so we print the whole
       * buffer at once as a full string
      */
      if ( fd == 1 ) putbuf( buf, size ); // STDOUT_FILENO

      /* ohterwise, if the file descriptor
       * is anything other than 0, which
       * means we are writing to an
       * opened file from the file system
      */
      else { /* writing to a file */

        /* retrieve the file map from the
         * file list of the current running
         * process using its file descriptor
         * and then store its file pointer
        */
        struct file *file = get_file_map( fd )->file;

        /* make sure the file was found,
         * otherwise return a status fail
         * denoted by -1 because we
         * can't write less than 0 characters
        */
        if ( file == NULL ) { f->eax = -1; break; }

        /* lock the file system to make
         * sure the file we are writing
         * into is not altered by
         * writing to it, or is not being
         * read from at the time for example.
         * and then store the size
         * of bytes written to be returned
         * to eax because we could have
         * written less than the expected
         * size and release the lock
        */
        lock_acquire( &file_lock );
        size = file_write( file, buf, size );
        lock_release( &file_lock );
      }

      /* return the size of bytes
       * written from either the terminal
       * or a file from the file system
      */
      f->eax = size;

      break;
    }

    case SYS_SEEK:
    {
      printf( "SYS_SEEK not yet implemented\n" );
      f->eax = -1;
      break;
    }

    case SYS_TELL:
    {
      printf( "SYS_TELL not yet implemented\n" );
      f->eax = -1;
      break;
    }

    case SYS_CLOSE:
    {
      /* retrieve the file descriptor
       * number of the opened file
       * that we want to close
      */
      int fd = *(int *)( esp + 4 );

      /* retrieve the file map from the
       * file list of the current running
       * process using its file descriptor
      */
      struct file_map *file_map = get_file_map( fd );

      /* make sure the file map was found,
       * otherwise abort
      */
      if ( file_map == NULL ) break;

      /* lock the file system to make
       * sure the file we are closing
       * is not altered by being used.
      */
      lock_acquire( &file_lock );
      file_close( file_map->file );
      lock_release( &file_lock );

      /* remove the file map from
       * the file list of the current
       * running process by removing
       * its list elem from the list
      */
      list_remove( &file_map->elem );

      /* once the file map is removed
       * from the file list, free its
       * data from memory, otherwise a
       * memory leak will happen, and we
       * will run out of memory of so many
       * files keep getting opened and closed
       * without being freed from memory
      */
      free( file_map );

      break;
    }

    default:
      printf( "syscall will not be implemented" );
      f->eax = -1;
      break;
  }
}
