#include "chatserver.hpp"
#include "json.hpp"
#include "encryption.hpp"
#include <functional>
#include <string>
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
    //初始化聊天服务器对象
ChatServer::ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg): _server(loop,listenAddr,nameArg){
    //注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
    //设置线程数目
    _server.setThreadNum(4);
}
//启动服务
void ChatServer::start(){
    _server.start();
}	

//链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn){
    //客户端断开
    if(!conn->connected()){
        //处理客户端异常退出。
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }  
}
//专门处理用户读写事件 的回调函数。
void ChatServer::onMessage (const TcpConnectionPtr& conn,
		Buffer* buffer,
		Timestamp time){
    string buf = buffer->retrieveAllAsString();
    // 接收到buf数据,解密。
    // AES解密
    // cout << "server:onMsg:" << buf << ":" << buf.size() << endl;
    string buf_unencry = aes_opt::aesDecrypt(buf);
    if (buf_unencry.empty()) {
        cout << "Server:onMsg:AES解码失败。" << endl;
    }
    //数据解码
    json js = json::parse(buf_unencry);
    //通过js["msgid"]获取handler.调用对应操作函数。
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //调用回调消息绑定的事件处理函数。
    msgHandler(conn,js,time);
}
