#ifndef USEROPERATOR_H
#define USEROPERATOR_H

#include "User.hpp"

//User表的数据操作类。增删改查。
class UserOperator{
public:
    //增加表项
    bool insert(User& user);
    //根据用户ID查询表项
    User query(int id);
    //更新用户信息。
    bool updateState(User user);
    //重置用户状态信息。
    void resetState();
};
#endif