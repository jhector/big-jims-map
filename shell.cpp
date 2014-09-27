#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

void error(const char *msg)
{
    write(1, msg, strlen(msg));
}

int32_t main(int32_t argc, char *argv[])
{
    return 0;
}
