//muduo两个主要类:
//TcpServer:编写服务器程序
//TcpClient:编写客户端程序。
//优点:将网络I/O代码与业务代码分离。
//业务:用户的链接与断开;用户的可读写事件。

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

#include <functional>						//~绑定器。
#include <iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;				//参数占位符

//基于muduo网络库开发服务器程序
//1.组合TcpServer对象
//2.创建EventLoop事件循环对象的指针
//3.明确TcpServer构造函数参数。输出ChatServer构造函数
//4.在当前服务器类的构造函数中注册处理链接的回调函数、处理读写事件的回调函数。
//5.设置合适的服务端线程数目。muduo库会自己分配IO线程和worker线程。

class ChatServer {
public:
	//事件循环-IP+PORT-服务器名称
	ChatServer(EventLoop* loop, const InetAddress& listenAddr, 
		const string& nameArg) :_server(loop, listenAddr, nameArg), _loop(loop)
	{
		//给服务器注册用户链接/断开的回调。
		_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));			//***理解为什么要bind
		//给服务器注册用户读写事件的回调。
		_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
		//设置服务器端的线程数 链接线程+工作线程数目。1 IO 3 worker
		_server.setThreadNum(4);
	}
	//开启事件循环
	void start() {
		_server.start();
	}
private:
	//专门处理用户链接的创建和断开 的回调函数。
	void onConnection(const TcpConnectionPtr& conn) {
		if (conn->connected()) {
			cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:online" << endl;
		}
		else {
			cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:offline" << endl;
			conn->shutdown();
			//_loop->quit();
		}
	}
	//专门处理用户读写事件 的回调函数。
	void onMessage (const TcpConnectionPtr& conn,
		Buffer* buffer,
		Timestamp time)										//时间信息
	{
		string buf = buffer->retrieveAllAsString();				//接收数据放到字符串
		cout << "recv data:" << buf << " time:" << time.toString() << endl;
		conn->send(buf);
	}
	TcpServer _server;				//1.
	EventLoop* _loop;				//2.epoll

};

int main() {
	EventLoop loop;					//epoll
	InetAddress addr("127.0.0.1", 6000);
	ChatServer server(&loop, addr, "ChatServer");
	server.start();					// listenfd 添加到epoll.
	loop.loop();					//以阻塞方式等待新用户链接/已链接用户的读写事件等。
	return 0;
}