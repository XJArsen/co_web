// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_INETADDRESS_H_
#define INCLUDE_INETADDRESS_H_
#include <arpa/inet.h>

class InetAddress {
   private:
    struct sockaddr_in addr;
    socklen_t addr_len;

   public:
    InetAddress();
    InetAddress(const char *ip, uint16_t port);
    ~InetAddress();
    sockaddr_in getAddr();
    socklen_t getAddr_len();
    void setInetAddr(sockaddr_in, socklen_t);
};
#endif
