#ifndef CHATSERVICE_H
#define CHATSERVIVE_H

#include <unordered_map>        //消息id<--->事件处理
#include <functional>
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include <mutex>

using namespace std;
using json = nlohmann::json;
using namespace muduo;
using namespace muduo::net;

#include "User.hpp"                             
#include "UserOperator.hpp"
#include "OfflineMessage.hpp"
#include "Friend.hpp"
#include "GroupOpt.hpp"
#include "redis.hpp"


//处理消息的事件回调方法类型。function<void(三参数:conn,js,time)>
using MsgHandler = function<void(const TcpConnectionPtr &conn,json &js,Timestamp time)>;

// 聊天服务器业务类
class ChatService
{
public:
    //静态成员函数 唯一实例 单例解口
    static ChatService* instance();
    //处理登录业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //处理注销业务
    void loginOut(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //创建群聊业务.创建AllGroup对象。生成sql表格,插入创建者身份。
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //加入群聊业务,根据群号加入sql. {"msgid","userid","groupid"}
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //群聊天业务,{"msgid","userid","groupid","msg"}
    void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //获取消息对应的处理器(方法).<MsgHandler>
    MsgHandler getHandler(int msgid);           
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //处理服务器异常,业务重置
    void reset();
    //从redis消息队列中得到订阅的消息。channel对应的用户在线:直接发送。否则存入离线消息表。
    //msg 是 别的 服务器上的用户 发给 本用户的消息内容.
    void handleRedisSubscribeMsg(int channel,string msg);
private:
    ChatService();
    //消息Id和其对应的业务处理方法。一开始已经定义好了。
    unordered_map<int,MsgHandler> _msgHandlerMap;
    //User表操作类对象。
    UserOperator _userModel;
    //OfflineMessage操作类对象。
    OfflineMessage _offmsgOpt;
    //Friend操作类对象
    Friend _friendOpt;
    //Group操作类对象
    GroupOpt _groupOpt;
    //存储在线用户的通信连接。随着用户上下线需要变动。多线程下考虑线程安全问题。--mutex
    unordered_map<int,TcpConnectionPtr> _userConnMap;
    //定义互斥锁保证线程安全
    mutex _connMutex;
    //Redis操作对象
    Redis _redis;
};

#endif