#include <stdio.h>
#include <syscall.h>

int
main( int argc, char **argv ) {
  int i;

  for ( i = 0; i < argc; i++ )
    printf( "%s ", argv[i] );
  printf( "\n" );

  return EXIT_SUCCESS;
}


// #include <stdio.h>
// #include <syscall.h>
// #include <string.h>
// int
// main( int argc, char **argv ) {
//   int i;
//   int f = open( argv[argc - 1] );
//   for ( i = 0; i < argc; i++ )
//     write( f, argv[i], strlen( argv[i] ) );
//   // printf( "\n" );

//   close( f );
//   return EXIT_SUCCESS;
// }
