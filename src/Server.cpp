#include "Socket.h"
#include "Server.h"
#include "Acceptor.h"
#include "Connection.h"
const short MAX_EVENTS = 1024;
const short READ_BUFFER = 1024;

Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr) {
    acceptor = new Acceptor(loop);
    std::function<void(Socket *)> cb =
        std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server() {
    delete acceptor;
}

void Server::newConnection(Socket *serv_socket) {
    Connection *connect = new Connection(loop, serv_socket);
    std::function<void(Socket *)> cb =
        std::bind(&Server::deleteConnection, this, std::placeholders::_1);

    connect->setDeleteConnectionCallback(cb);
    connections[serv_socket->getFd()] = connect;
}
void Server::deleteConnection(Socket *serv_socket) {
    Connection *connect = connections[serv_socket->getFd()];
    connections.erase(serv_socket->getFd());
    delete connect;
}