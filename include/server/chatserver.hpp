#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;
class ChatServer
{
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
    //启动服务
    void start();	
private:
    //链接相关信息的回调函数
    void onConnection(const TcpConnectionPtr& conn);
	//专门处理用户读写事件 的回调函数。
	void onMessage (const TcpConnectionPtr& conn,
		Buffer* buffer,
		Timestamp time);	
    TcpServer _server;                  //组合的muduo网络库对象。
    EventLoop *_loop;                   //事件循环对象 
};

#endif