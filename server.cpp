#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "util.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Epoll.h"
const short MAX_EVENTS = 1024;
const short READ_BUFFER = 1024;
void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main() {
    Socket *serv_socket = new Socket();

    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_socket->bind(serv_addr);
    serv_socket->listen();
    serv_socket->setnonblocking();

    Epoll *ep = new Epoll();

    ep->addFd(serv_socket->getFd(), EPOLLIN | EPOLLET);

    while (true) {
        std::vector<epoll_event> events = ep->poll();
        int nfds = events.size();
        errif(nfds == -1, "epoll wait error");
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == serv_socket->getFd()) {
                InetAddress *client_addr = new InetAddress();
                Socket *client_sock =
                    new Socket(serv_socket->accept(client_addr));

                printf("new client fd %d! IP : %s Port: %d\n",
                       client_sock->getFd(),
                       inet_ntoa(client_addr->addr.sin_addr),
                       ntohs(client_addr->addr.sin_port));

                ep->addFd(client_sock->getFd(), EPOLLIN | EPOLLET);
                client_sock->setnonblocking();
            } else if (events[i].events & EPOLLIN) {
                while (true) {
                    char buf[READ_BUFFER];
                    bzero(&buf, sizeof buf);
                    ssize_t read_bytes =
                        read(events[i].data.fd, buf, sizeof(buf));
                    if (read_bytes > 0) {
                        printf("message from client fd %d: %s\n",
                               events[i].data.fd, buf);
                        write(events[i].data.fd, buf, sizeof(buf));
                    } else if (read_bytes == 0) {
                        printf("EOF, client fd %d disconnected",
                               events[i].data.fd);
                        close(events[i].data.fd);
                        break;
                    } else if (read_bytes == -1) {
                        if (errno == EINTR) {
                            printf("continue reading");
                            continue;
                        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            printf("finish reading once, errno: %d\n", errno);
                            break;
                        }
                    }
                }
            } else {
                printf("----");
            }
        }
    }
    delete serv_socket;
    delete serv_addr;
    return 0;
}
