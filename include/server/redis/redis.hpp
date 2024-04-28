#ifndef REDIS_H
#define REDIS_H
#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;
class Redis
{
public:
    Redis();
    ~Redis();
    //连接redis服务器
    bool connect();
    //向redis指定channel发布消息
    bool publish(int channel,string message);
    //从redis指定channel订阅消息
    bool subscribe(int channel);
    //从指定channel取消订阅
    bool unsubscribe(int channel);
    //在独立线程中接收订阅通道的消息
    void observer_channel_msg();
    //初始化向业务层上报通道消息的回调对象
    void init_notify_handler(function<void(int,string)> fn);
private:
    //subscribe 与 publish 不能在一个上下文去处理:因为 subscribe会阻塞。
    //hiredis同步上下文对象,负责publish
    redisContext *_publish_context;
    //hiredis同步上下文对象,负责subscribe
    redisContext *_subscribe_context;
    //回调 受到订阅消息,上报service.
    function<void(int,string)> _notify_message_handler;
};

#endif
