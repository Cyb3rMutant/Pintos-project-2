#include <stdio.h>
#include <string.h>

//To Run:
//pintos -q rm shellcode;pintos -q rm hacc;pintos -q rm cat;cd ../../examples/;make;cd ../userprog/build/;pintos -v -p ../../examples/shellcode -a shellcode -p  ../../examples/echo.c -a hacc -p ../../examples/cat -a cat -- -f -q;pintos -q run 'shellcode'
//pintos -q run 'cat hacc'

//To run with gdb:
//Open two terimnals
//terminal 1 'run pintos -q --gdb -- run 'shellcode''
//terminal 2 'pintos-gdb kernel.o'
//terminal 2 'debugpintos'
//terminal 2 set a breakpoint at syscall_handler 'b syscall_handler' or where u want breakpoint
//terminal 1 run the exploit

//GDB commands: c = continue, ni = next instruction, b = breakpoint

char shellcode[] = "\x6A\x00\x68\x68\x61\x63\x63\x54\x6A\x06\xCD\x30\x89\xC2\x6A\x00\x68\x68\x69\x79\x61\x89\xE1\x6A\x04\x51\x52\x6A\x09\xCD\x30";



int main( void )
{
   /* main from https://shell-storm.org/shellcode/files/shellcode-237.html */
  
   void  (*fp) (void);
   fp = (void *) shellcode;
   printf ("%d bytes\n", strlen(shellcode));
   fp();

   return 0;
}