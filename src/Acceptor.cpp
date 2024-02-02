// #include "Acceptor.h"
// #include "cstdio"
// Acceptor::Acceptor(EventLoop *_loop) : loop(_loop), acceptChannel(nullptr) {
//     sock = new Socket();
//     addr = new InetAddress("127.0.0.1", 8888);
//     sock->bind(addr);
//     sock->listen();
//     acceptChannel = new Channel(loop, sock->getFd());
//     std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
//     sock->setnonblocking();
//     acceptChannel->setReadCallback(cb);
//     acceptChannel->enableReading();
//     acceptChannel->setUseThreadPool(false);
//     delete addr;
// }

// Acceptor::~Acceptor() {
//     delete sock;
//     delete acceptChannel;
// }
// void Acceptor::acceptConnection() {
//     InetAddress *client_addr = new InetAddress();
//     Socket *clnt_sock = new Socket(sock->accept(client_addr));
//     printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(),
//            inet_ntoa(client_addr->getAddr().sin_addr), ntohs(client_addr->getAddr().sin_port));

//     clnt_sock->setnonblocking();
//     newConnectionCallback(clnt_sock);
//     delete client_addr;
// }
// void Acceptor::setNewConnectionCallback(std::function<void(Socket *)> _cb) {
//     newConnectionCallback = _cb;
// }