#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include <vector>
using namespace std;
using namespace muduo;
//唯一实例 单例接口
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}
//注册消息以及对应的回调操作
ChatService::ChatService(){
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGIN_OUT,std::bind(&ChatService::loginOut,this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});
    _msgHandlerMap.insert({CTE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GRP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    //连接redis 服务器
    if(_redis.connect()){
        //redis class通过init_notify_handler站好位置了,这里填入handleRedisSubscribeMsg绑定即可.
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMsg,this,_1,_2));
    }
}

MsgHandler ChatService::getHandler(int msgid){
    //记录错误日志:msgid没有对应的事件处理回调。
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()){
        //返回一个默认处理器。[=]拷贝外部变量?
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp time){
            LOG_ERROR << "msgid:" << msgid << " can not find handler.";
        };
    }else{
        return _msgHandlerMap[msgid];
    }
}

//处理登录业务
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp time){
    LOG_INFO << "do login service.";
    int id = js["id"].get<int>();
    string pwd = js["password"];
    
    User user = _userModel.query(id);
    if(user.getId() == id && user.getPassword() == pwd){
        if(user.getState() == "online"){
            //用户已经登录。
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;                      //检验消息。=0无误
            response["errmsg"] = "该账户已经登录。";
            conn->send(response.dump());
        }else{
            //_userConnMap建立用户与服务器的长连接.记录。
            //利用只能指针,自动加解锁。
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({user.getId(),conn});
            }
            //登录成功,订阅channel id的消息。
            _redis.subscribe(id);                          
            //登录成功,state->online.
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;                      //检验消息。=0无误
            response["id"] = user.getId();
            response["name"] = user.getName();
            //登录成功,1.查询用户的好友信息并返回。
            vector<User> userVec = _friendOpt.query(user.getId());
            if(!userVec.empty()){
                vector<string> vectmp;
                for(User &user:userVec){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vectmp.push_back(js.dump());
                } 
                response["friends"]=vectmp;
            }
            //2.查询用户的群组信息并返回。groups:id-groupname-groupdesc-users
            vector<AllGroup> groupuserVec = _groupOpt.queryGroups(id);
            if(!groupuserVec.empty()){
                vector<string> groupV;
                for(AllGroup &group:groupuserVec){
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getGroupname();
                    grpjson["groupdesc"] = group.getGroupdesc();
                    vector<string> userV;
                    for(GroupUser &user:group.getUsers()){
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["grouprole"] = user.getGrouprole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }
                response["groups"] = groupV;
            } 

            //3.查询用户是否有离线消息。有的话放入json.
            vector<string> msgVec = _offmsgOpt.query(user.getId());
            if(!msgVec.empty()){
                response["offlinemsg"] = msgVec;
                //读取离线消息后,移除离线消息。
                _offmsgOpt.remove(user.getId());
            }

            conn->send(response.dump());
        }
    }else{
        //登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;                      
        response["errmsg"] = "用户名或密码错误。";
        conn->send(response.dump());
    }
}
void ChatService::loginOut(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end()){
            _userConnMap.erase(it);
        }
    }
    //用户注销~下线。在redis取消订阅通道。
    _redis.unsubscribe(userid);
    //更新用户状态信息
    User user(userid,"","","offline");
    _userModel.updateState(user);
}


//处理注册业务 ~name - password
void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp time){
    //LOG_INFO << "do reg service.";
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool status = _userModel.insert(user);
    if(status){
        //注册成功-回应客户端消息
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;                      //检验消息。=0无误
        response["id"] = user.getId();
        conn->send(response.dump());
    }else{
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;                     
        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int dstId = js["dstid"].get<int>();
    //LOG_INFO << "do oneChat Service.";
    //LOG_INFO << js["msg"].dump();
    //dstId对应的连接可能会在发送的时候被删除。需要放在mutex域中。
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(dstId);
        if(it != _userConnMap.end()){ 
            it->second->send(js.dump());
            return;
        }
    }
    //查询 dstid 是否在线.跨服务器。
    User user = _userModel.query(dstId);
    if(user.getState() == "online"){
        _redis.publish(dstId,js.dump());
        //cout << "-------" << js.dump() << endl;
        return;
    }
    //离线消息。存储
    _offmsgOpt.insert(dstId,js.dump());
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin();it != _userConnMap.end(); ++it){
            if(it->second == conn){
                //从map表删除用户的连接信息。
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    //相当于下线。取消订阅。
    _redis.unsubscribe(user.getId());

    //更新用户状态信息
    if(user.getId()!=-1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}

//msgid,id,friendid.
void ChatService::addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    //写入friend表中。
    _friendOpt.addRelation(userid,friendid);
}

void ChatService::reset(){
    //online重置为offline.
    _userModel.resetState();
}

//创建群聊业务.
void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time){
    //注册AllGroup表以及GroupUser表。
    int userid = js["userid"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    AllGroup group(-1,name,desc);
    if(_groupOpt.createGroup(group)){
        //存储群组创建人信息
        _groupOpt.addGroup(userid,group.getId(),"creator");
    }
}

//加入群聊业务
void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupOpt.addGroup(userid,groupid,"nomal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> dstId = _groupOpt.queryGroupUsers(userid,groupid);

    //dstId对应的连接可能会在发送的时候被删除。需要放在mutex域中。
    lock_guard<mutex> lock(_connMutex);
    for(int i=0;i<dstId.size();i++)
    {
        auto it = _userConnMap.find(dstId[i]);
        if(it != _userConnMap.end()){ 
            it->second->send(js.dump());
            return;
        }else{
            //查询 dstId 是否在线。
            User user = _userModel.query(dstId[i]);
            if(user.getState() =="online"){
                _redis.publish(dstId[i],js.dump());
            }else{
                //离线消息。存储
                _offmsgOpt.insert(dstId[i],js.dump());
            }
        }
    }
}

void ChatService::handleRedisSubscribeMsg(int channel,string msg){
    lock_guard<mutex> lock(_connMutex);             //防止取消息途中channel对应用户下线。
    auto it = _userConnMap.find(channel);
    if(it != _userConnMap.end()){
        cout << "publish to server:" << channel << ",msg:" << msg <<  endl;
        it->second->send(msg);
        return;
    }
    //存储该用户的离线消息
    _offmsgOpt.insert(channel,msg);
}