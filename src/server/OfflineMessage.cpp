#include "OfflineMessage.hpp"
#include "db.h"

//存储用户离线消息 id,msg
void OfflineMessage::insert(int Userid,string Usermsg){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into OfflineMessage values(%d,'%s')",
            Userid,Usermsg.c_str());
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}
//删除用户离线消息
void OfflineMessage::remove(int Userid){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"delete from OfflineMessage where userid=%d",Userid);
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
        }
    }
}
//查询用户离线消息 可能不止一个->vector<string>
vector<string> OfflineMessage::query(int userid){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"select message from OfflineMessage where userid=%d",userid);
    vector<string> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr){
                vec.push_back(row[0]);
            }            
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
