#include <errno.h>
#include <string.h>
#include <stdio.h>

int main() {
    char buf[280];
    char* s = strerror_r(ENOENT, buf, 280);
    printf("%s", s);
    return 0;
}
