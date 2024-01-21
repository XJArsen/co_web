#include "Server.h"
#include "EventLoop.h"
const short MAX_EVENTS = 1024;
const short READ_BUFFER = 1024;

int main() {
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    loop->loop();
    return 0;
}
