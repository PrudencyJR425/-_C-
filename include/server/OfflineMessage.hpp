#ifndef OFFLINEMESSAGE_H
#define OFFLINEMESSAGE_H
#include<string>
#include<vector>
using namespace std;
//OfflineMessage表操作。
class OfflineMessage{
public:
//存储用户离线消息 id,msg
void insert(int Userid,string Usermsg);
//删除用户离线消息
void remove(int Userid);
//查询用户离线消息 可能不止一个->vector<string>
vector<string> query(int userid);
};
#endif