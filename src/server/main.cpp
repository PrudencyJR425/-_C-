#include "chatserver.hpp"
#include "chatservice.hpp"
#include<iostream>
#include<signal.h>
using namespace std;
//服务器ctrl+c结束后,重置User状态信息。
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char *argv[]){
    if(argc < 3){
        cout << "error input." << endl;
    }
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr(argv[1],atoi(argv[2]));
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    return 0;
}
