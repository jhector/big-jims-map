#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

void handler(int sig)
{
    exit(0);
}

void error(const char *msg)
{
    write(1, msg, strlen(msg));
}

uint32_t read_until(char term, char *out, uint32_t max)
{
    uint32_t pos = 0;
    char byte = 0;

    while (read(0, &byte, 1) == 1 && byte != term) {
        if (pos >= max)
            return pos;

        out[pos] = byte;
        pos++;
    }

    return pos-1;
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
    char request[128] = {0};
    char host[128] = {0};
    char tmp[10] = {0};

    signal(SIGALRM, handler);
    alarm(30);

    read_until('\n', request, sizeof(request)-1);
    if (!request) {
        err = "Couldn't read request line\n";
        goto fail;
    }

    read_until('\n', host, sizeof(host)-1);
    if (!host) {
        err = "Couldn't read host\n";
        goto fail;
    }

    read_until('\n', tmp, sizeof(tmp)-1);

    try {
        handle_request(request, host);
    } catch (const char *e) {
        err = e;
        goto fail;
    }

    return 0;

fail:
    error(err);
    return 1;
}
