#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include "util.h"
#include "InetAddress.h"
#include "Socket.h"
const short READ_BUFFER = 1024;

int main() {
    Socket *client_socket = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    client_socket->listen();

    while (true) {
        char buf[READ_BUFFER];
        bzero(&buf, sizeof buf);
        scanf("%s", buf);
        ssize_t write_bytes = write(client_socket->getFd(), buf, sizeof(buf));
        if (write_bytes == -1) {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        bzero(&buf, sizeof buf);
        ssize_t read_bytes = read(client_socket->getFd(), buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from server: %s\n", buf);
        } else if (read_bytes == 0) {
            printf("server socker disconnected");
            break;
        } else if (read_bytes == -1) {
            close(client_socket->getFd());
            errif(true, "socker read error");
        }
    }

    return 0;
}
