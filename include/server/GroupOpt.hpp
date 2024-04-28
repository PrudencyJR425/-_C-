#ifndef GROUPOPT_H
#define GROUPOPT_H
#include "AllGroup.hpp"

//进行群组操作 创建-加入-查询群信息vec<AllGroup>-查询群成员id用于群聊vec<int>
class GroupOpt{
public:
    //创建群。传入Allgroup对象。创建sql-group表格,创建成功会自动填入group Id。
    bool createGroup(AllGroup &group);
    //加入群.传入userid-groupid,role.函数会更新groupid对应的GroupUser表格。
    bool addGroup(int userid,int groupid,string role);
    //查询用户所在群信息.传入userid.
    //先根据userid查群组信息。
    //再根据群组信息，找所有userid.每个AllGroup维护一个vector,存放用户id.
    vector<AllGroup> queryGroups(int userid);
    //传入userid,groupid.根据指定groupid查询除了userid的用户id列表。。用于向群组用户发送消息。
    vector<int> queryGroupUsers(int userid,int groupid);
};

#endif