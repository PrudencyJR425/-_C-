#include "GroupOpt.hpp"
#include "db.h"

//创建群。传入Allgroup对象。创建sql-group表格,创建成功会自动填入group Id。
bool GroupOpt::createGroup(AllGroup &group){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into AllGroup(groupname,groupdesc) values('%s','%s')",
            group.getGroupname().c_str(),group.getGroupdesc().c_str());
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
//加入群.传入groupid,userid,role.函数会更新groupid对应的GroupUser表格。
bool GroupOpt::addGroup(int userid,int groupid,string role){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into GroupUser(groupid,userid,grouprole) values(%d,%d,'%s')",
            groupid,userid,role.c_str());
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql))
            return true;
    }
    return false;
}
//查询用户所在群信息.传入userid.
//先根据userid查群组信息。
//再根据群组信息，找所有userid.每个AllGroup维护一个vector,存放用户id.
vector<AllGroup> GroupOpt::queryGroups(int userid){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from AllGroup a \
            inner join GroupUser b on a.id=b.groupid where b.userid=%d",userid);
    vector<AllGroup> Gpvec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            //查所有群信息
            while((row = mysql_fetch_row(res))!=nullptr){
                AllGroup group;
                group.setId(atoi(row[0]));
                group.setGroupname(row[1]);
                group.setGroupdesc(row[2]);
                Gpvec.push_back(group);
            }            
            mysql_free_result(res);
        }
    }
    //查询群组用户信息。
    for(AllGroup& Curgroup:Gpvec){
        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from User a \
                inner join GroupUser b on b.userid = a.id where b.groupid=%d",Curgroup.getId());
        MySQL mysql;
        if(mysql.connect()){
            MYSQL_RES *res = mysql.query(sql);
            if(res != nullptr){
                MYSQL_ROW row;
                //查所有群信息
                while((row = mysql_fetch_row(res))!=nullptr){
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setGrouprole(row[3]);
                    Curgroup.getUsers().push_back(user);
                }            
                mysql_free_result(res);
            }        
        }
    }
    return Gpvec;
}

//传入userid,groupid.根据指定groupid查询除了userid的用户id列表。。用于向群组用户发送消息。
vector<int> GroupOpt::queryGroupUsers(int userid,int groupid){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"select userid from GroupUser where groupid=%d and userid != %d",groupid,userid);
    vector<int> idVec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr){
                idVec.push_back(atoi(row[0]));
            }            
            mysql_free_result(res);
        }
    }
    return idVec;    
}

