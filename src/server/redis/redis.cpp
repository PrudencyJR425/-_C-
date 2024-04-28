#include "redis.hpp"
#include <iostream>
Redis::Redis():_publish_context(nullptr),_subscribe_context(nullptr){

}
Redis::~Redis(){
    if(_publish_context!=nullptr){
        redisFree(_publish_context);
    }
    if(_subscribe_context!=nullptr){
        redisFree(_subscribe_context);
    }
}

//连接redis服务器。服务器为单例模式->创建一个thread阻塞等待subscribe reply.
bool Redis::connect(){
    //负责publish发布消息的上下文连接
    _publish_context = redisConnect("127.0.0.1",6379);
    if(nullptr == _publish_context){
        cerr << "connect redis failed." << endl;
        return false;
    }
    _subscribe_context = redisConnect("127.0.0.1",6379);
    if(nullptr == _subscribe_context){
        cerr << "connect redis failed." << endl;
        return false;
    }    
    //在单独线程中监听通道事件。有消息给业务层进行上报。
    thread t([&](){
        observer_channel_msg();
    });
    t.detach();
    cout << "connect redis-server success." << endl;
    return true;
}

//向redis指定channel发布消息
bool Redis::publish(int channel,string message){
    //传入 msg.c_str()避免乱码问题.
    redisReply *reply = (redisReply*) redisCommand(_publish_context,"PUBLISH %d %s",channel,
                        message.c_str());
    cout << "redis::public:" << message.c_str() << endl; 
    if(nullptr == reply){
        cerr << "Publish command failed." << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}
//redisCommand = redisAppendCommand(读取命令) + redisBufferWrite(写入缓冲) + redisGetReply.

//从redis指定channel订阅消息
bool Redis::subscribe(int channel){
    //subscribe 命令会造成线程阻塞等待通道的消息。本函数只订阅通道,而不接收通道消息。
    //通道消息的接收专门在 observer_channel_msg 函数的独立线程中进行。
    //只负责发送命令,而不阻塞接收redis server的响应消息。
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context,"SUBSCRIBE %d",channel)){
        cerr << "subscribe command failed." << endl;
        return false;
    }
    //redisBufferWriter循环发送缓冲区。直到缓冲区数据发送完毕。done置1.
    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context,&done)){
            cerr << "subscribe command failed." << endl;
            return false;
        }
    }
    return true;
}
//从指定channel取消订阅
bool Redis::unsubscribe(int channel){
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context,"UNSUBSCRIBE %d",channel)){
        cerr << "unsubscribe command failed." << endl;
        return false;
    }
    //redisBufferWriter循环发送缓冲区。直到缓冲区数据发送完毕。done置1.
    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context,&done)){
            cerr << "unsubscribe command failed." << endl;
            return false;
        }
    }
    return true;
}
//在独立线程中接收订阅通道的消息:阻塞等待。调用回调操作,向业务层上报channel id+msg.
void Redis::observer_channel_msg(){
    redisReply *reply = nullptr;
    while(REDIS_OK == redisGetReply(this->_subscribe_context,(void **)&reply)){
        //订阅的消息是一个三元素数组 element[0]:sign,element[1]:channel,element[2]:msg;
        if(reply!=nullptr && reply->element[2]!=nullptr && reply->element[2]->str != nullptr){
            //向业务层上报通道上发生的消息。
            cout << "redis::observe:" << reply->element[2]->str << endl;
            _notify_message_handler(atoi(reply->element[1]->str),reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    cerr << ">>>>>>>>>>observer_channel_msg quit<<<<<<<<<<<" << endl;
}

//初始化向业务层上报通道消息的回调对象
void Redis::init_notify_handler(function<void(int,string)> fn){
    this->_notify_message_handler = fn;
}

