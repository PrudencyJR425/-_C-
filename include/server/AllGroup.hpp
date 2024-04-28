#ifndef ALLGROUP_H
#define ALLGROUP_H
#include <string>
#include <vector>
#include "GroupUser.hpp"

using namespace std;
//群属性类:id<int>,groupname<string>,groupdesc<string>,gpUservec<vector<GroupUser>>
class AllGroup{
public:
    AllGroup(int id=-1,string groupname="",string groupdesc=""){
        this->id = id;
        this->groupname = groupname;
        this->groupdesc = groupdesc;
    }
    void setId(const int id){
        this->id = id;
    }
    void setGroupname(const string groupname){
        this->groupname = groupname;
    }
    void setGroupdesc(const string groupdesc){
        this->groupdesc = groupdesc;
    }
    int getId(){
        return this->id;
    }
    string getGroupname(){
        return this->groupname;
    }
    string getGroupdesc(){
        return this->groupdesc;
    }
    vector<GroupUser>& getUsers(){
        return this->gpUservec;
    }
private:
    //组id
    int id;
    //组名称
    string groupname;
    //组描述
    string groupdesc;
    //组内成员表
    vector<GroupUser> gpUservec;
};
#endif