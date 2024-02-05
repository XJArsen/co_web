#include "InetAddress.h"
#include <cstring>

InetAddress::InetAddress() : addr_len(sizeof addr) {
    bzero(&addr, sizeof addr);
}
InetAddress::~InetAddress() {
}
InetAddress::InetAddress(const uint32_t _ip, uint16_t _port) : addr_len(sizeof addr) {
    bzero(&addr, sizeof addr);
    addr.sin_addr.s_addr = htonl(_ip);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
}
InetAddress::InetAddress(const char *_ip, uint16_t _port) : addr_len(sizeof addr) {
    bzero(&addr, sizeof addr);
    addr.sin_addr.s_addr = inet_addr(_ip);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
}
sockaddr_in InetAddress::getAddr() {
    return addr;
}
socklen_t InetAddress::getAddr_len() {
    return addr_len;
}
void InetAddress::setInetAddr(sockaddr_in _adrr, socklen_t _adrr_len) {
    addr = _adrr;
    addr_len = _adrr_len;
}
