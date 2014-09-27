#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

void error(const char *msg)
{
    write(1, msg, strlen(msg));
}

char* read_until(char term, uint32_t max)
{
    char *content = (char*)calloc(1, max);
    uint32_t pos = 0;
    char byte = 0;

    if (!content)
        return NULL;

    while (read(0, &byte, 1) == 1 && byte != term) {
        if (pos >= max)
            return content;

        content[pos] = byte;
        pos++;
    }

    return content;
}

int32_t main(int32_t argc, char *argv[])
{
    const char *err = NULL;
    char *request = NULL;

    request = read_until('\n', 128);
    if (!request) {
        err = "Couldn't read request line\n";
        goto fail;
    }

    return 0;

fail:
    free(request);

    error(err);
    return 1;
}
