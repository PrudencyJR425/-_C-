#ifndef FRIEND_H
#define FRIEND_H
#include <iostream>
#include "db.h"
#include <vector>
#include "User.hpp"
using namespace std;

//维护好友信息表的操作类
class Friend{
public:
    //添加好友关系。
    void addRelation(int userid,int friendid);
    //返回用户的好友列表。返回friendid 以及 对应名称,在线与否。 两个表联合查询
    vector<User> query(int userid);
};

#endif