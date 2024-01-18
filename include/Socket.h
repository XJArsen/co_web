#ifndef SOCKET_H
#define SOCKET_H
class InetAddress;
class Socket {
   private:
    int fd;

   public:
    Socket();
    Socket(int);
    ~Socket();

    void bind(InetAddress*);
    void connect(InetAddress*);
    void listen();
    int accept(InetAddress*);
    int getFd();
    void setnonblocking();
};
#endif