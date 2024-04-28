#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "User.hpp"

//GroupUser表操作类。继承自User类。复用User其他信息。
//id,name,state,grouprole
class GroupUser:public User{
public:
    void setGrouprole(string role){
        this->grouprole = role;
    }
    string getGrouprole(){
        return this->grouprole;
    }
private:
    string grouprole;
};
#endif