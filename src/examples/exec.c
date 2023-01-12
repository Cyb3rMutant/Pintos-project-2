#include <syscall.h>

int main() {
    exec( "echo x y z" );
    return 0;
}
