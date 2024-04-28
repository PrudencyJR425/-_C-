#ifndef PUBLIC_H
#define PUBLIC_H
/*
server 和 client 的公共文件。
*/

/*
一对一聊天的json消息
msgid:ONE_CHAT_MSG 5
srcid:1
srcname:"zhangsan"
dstid:2                     //查id对应的conn表。在线->发送。否则存入离线消息表。
msg:"xxx"
*/

// msgId
enum EnMsgType{
    LOGIN_MSG = 1,          //Login
    LOGIN_OUT,              //注销
    REG_MSG,                //reg
    REG_MSG_ACK,            //reg响应消息
    LOGIN_MSG_ACK,          //Login响应消息
    ONE_CHAT_MSG,           //聊天消息
    ADD_FRIEND_MSG,         //添加好友消息
    CTE_GROUP_MSG,          //创建群聊
    ADD_GROUP_MSG,          //加入群聊
    GRP_CHAT_MSG            //群聊天
};
#endif