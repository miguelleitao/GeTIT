// receiver.c
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SOCKET_PATH "/tmp/getit_l1.sock"
#define BUF_SIZE 128

int main(void) {
    int fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH); // remove se já existir

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    printf("Receiver à escuta em %s\n", SOCKET_PATH);

    while (1) {
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n < 0) {
            perror("recv");
            break;
        }

        buf[n] = '\0';
        printf("Recebido: %s\n", buf);
    }

    close(fd);
    unlink(SOCKET_PATH);
    return 0;
}
