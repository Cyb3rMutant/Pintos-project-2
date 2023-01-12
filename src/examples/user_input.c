#include <syscall.h>
#include <stdio.h>

int main() {
    char buf[6]="Hello\0";
    read(STDIN_FILENO, &buf, 5);
    printf("hello %s", buf);
    return 0;
}
