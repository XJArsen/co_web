// #include "Server.h"
// #include "Acceptor.h"
// #include "Connection.h"
// #include "Socket.h"

// Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr) {
//     acceptor = new Acceptor(loop);
//     std::function<void(Socket *)> cb =
//         std::bind(&Server::newConnection, this, std::placeholders::_1);
//     acceptor->setNewConnectionCallback(cb);
// }

// Server::~Server() {
//     delete acceptor;
// }

// void Server::newConnection(Socket *serv_socket) {
//     if (serv_socket->getFd() != -1) {
//         Connection *connect = new Connection(loop, serv_socket);
//         std::function<void(int)> cb =
//             std::bind(&Server::deleteConnection, this, std::placeholders::_1);

//         connect->setDeleteConnectionCallback(cb);
//         connections[serv_socket->getFd()] = connect;
//     }
// }
// void Server::deleteConnection(int sockfd) {
//     if (sockfd != -1) {
//         if (connections.count(sockfd)) {
//             Connection *connect = connections[sockfd];
//             connections.erase(sockfd);
//             delete connect;
//         }
//     }
// }