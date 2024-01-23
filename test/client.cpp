#include <unistd.h>
#include <cstdio>
#include <cstring>
#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"
#include "util.h"
const short READ_BUFFER = 1024;

int main() {
    Socket *sock = new Socket();
    InetAddress *addr = new InetAddress("127.0.0.1", 8888);
    sock->connect(addr);

    int sockfd = sock->getFd();

    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();

    while (true) {
        char buf[READ_BUFFER];
        sendBuffer->getline();
        ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if (write_bytes == -1) {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        int already_read = 0;
        while (true) {
            bzero(&buf, sizeof buf);
            ssize_t read_bytes = read(sock->getFd(), buf, sizeof(buf));
            if (read_bytes > 0) {
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if (read_bytes == 0) {
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            if (already_read >= sendBuffer->size()) {
                printf("message from server: %s\n", readBuffer->c_str());
                break;
            }
        }
        readBuffer->clear();
    }
    delete sock;
    delete addr;
    return 0;
}
