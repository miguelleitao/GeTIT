// sender.c
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SOCKET_PATH "/tmp/getit_l1.sock"

int main(void) {
    int fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    const char *msg = "hello from sender";

    if (sendto(fd, msg, strlen(msg), 0,
               (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("sendto");
        return 1;
    }

    printf("Mensagem enviada\n");

    close(fd);
    return 0;
}
