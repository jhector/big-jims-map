#include <sys/stat.h>
#include <fcntl.h>
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

void handle_request(char *req, char *host)
{
    char *iter = req;
    char *file = NULL;

    if (strcmp(host, "Host: localhost"))
        throw "Invalid host\n";

    if (strncmp("GET ", iter, 4))
        throw "Method not supported\n";

    iter += 4;
    file = iter;

    if (!(iter = strchr(file, ' ')))
        throw "Invalid request\n";

    *iter = 0x0;
    iter++;

    if (strcmp(iter, "HTTP/1.1"))
        throw "Unknown protocol\n";

    if (strstr(file, "flag"))
        throw "Sorry, can't read that\n";

    int32_t fd = open(file, O_RDONLY);
    if (fd < 0)
        throw "Can't open file\n";

    char content[128] = {0};
    uint32_t size = 0;

    while ((size = read(fd, content, sizeof(content)-1)) > 0) {
        write(1, content, size);
        bzero(content, sizeof(content));
    }

    close(fd);
}

int32_t main(int32_t argc, char *argv[])
{
    const char *err = NULL;
    char *request = NULL;
    char *host = NULL;
    char *tmp = NULL;

    request = read_until('\n', 128);
    if (!request) {
        err = "Couldn't read request line\n";
        goto fail;
    }

    host = read_until('\n', 128);
    if (!host) {
        err = "Couldn't read host\n";
        goto fail;
    }

    tmp = read_until('\n', 10);

    try {
        handle_request(request, host);
    } catch (const char *e) {
        err = e;
        goto fail;
    }

    return 0;

fail:
    free(request);
    free(host);
    free(tmp);

    error(err);
    return 1;
}
