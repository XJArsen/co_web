
cmake_minimum_required(VERSION 3.22.1)

project(webServer)

include_directories(../include)
set(src_server ./server.cpp ./Socket.cpp ./InetAddress.cpp ./util.cpp ./Epoll.cpp ./Channel.cpp ./EventLoop.cpp ./Server.cpp ./Acceptor.cpp ./Connection.cpp)
set(src_client ./client.cpp ./Socket.cpp ./InetAddress.cpp ./util.cpp)
add_executable(server ${src_server})
add_executable(client ${src_client})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin)
