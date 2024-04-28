#include "json.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "public.hpp"
#include <string>
#include "User.hpp"
#include "AllGroup.hpp"
#include <thread>
#include <vector>
#include <unordered_map>
using namespace std;
using json = nlohmann::json;

User _currentUser;
vector<User> _currentUserFriendsList;
vector<AllGroup> _currentUserGroupsList;
bool isMainMenuRunning = false;

void help(int a = 0,string b = "");
void chat(int,string);
void addfriend(int,string);
void creategroup(int,string);
void addgroup(int,string);
void groupchat(int,string);
void loginout(int,string);

//注册系统需要的命令列表
unordered_map<string,string> commandMap = {
    {"help","显示所有支持的命令,格式 help"},
    {"chat","一对一聊天,格式 chat:friendid:message"},
    {"addfriend","添加好友,格式 addfriend:friendid"},
    {"creategroup","创建群组,格式 creategroup:groupname:groupdesc"},
    {"addgroup","加入群组,格式 addgroup:groupid"},
    {"groupchat","群聊,格式 groupchat:groupid:message"},
    {"loginout","注销,格式 loginout"}
};


//注册系统需要的命令处理。
unordered_map<string,function<void(int,string)>> commandHandlerMap = {
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};


void ShowCurrentUserData(){
    cout << "=================login user===============" << endl;
    cout << "current login user => id:" << _currentUser.getId() << " name:" << _currentUser.getName() << endl;
    if(!_currentUserFriendsList.empty()){
        cout << "-----------------Friend List-----------------" <<endl;
        for(User &user:_currentUserFriendsList){
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    
    if(!_currentUserGroupsList.empty()){
        cout << "-----------------Group List---------------" << endl;
        for(AllGroup& group:_currentUserGroupsList){
            cout << group.getId() << "GroupName:"<< group.getGroupname() << " GroupDesc:" << group.getGroupdesc() << endl;
            cout << "-------------Group Member List------------" << endl;
            for(GroupUser &gpuser:group.getUsers()){
                cout <<gpuser.getId() << " " << gpuser.getName() << " " << gpuser.getState() << " "<< gpuser.getGrouprole() << endl;
            }    
        }
    }
    cout << "==========================================" << endl;
}
//接收线程
void readTaskHandler(int clientfd){
    for(;;){
        char buffer[1024] = {0};
        int len = recv(clientfd,buffer,1024,0);
        if(-1==len||0==len){
            close(clientfd);
            exit(-1);
        }
        //cout << "buffer---" << buffer << endl;
        json js = json::parse(buffer);
        if(ONE_CHAT_MSG == js["msgid"].get<int>()){
            cout << "私聊消息:"<<js["time"].get<string>() << " [" << js["id"] << "] " << 
            js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        if(GRP_CHAT_MSG == js["msgid"].get<int>()){
            cout << "群聊消息:"<<js["time"].get<string>() << " [" << js["userid"] << "] " << 
            js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
    }   
}

//获取系统时间。
string getCurrentTime(){
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date,"%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900,(int)ptm->tm_mon+1,(int)ptm->tm_mday,
            (int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
    return std::string(date);
}

void mainMenu(int clientfd){
    help();                         //显示系统命令。
    char buffer[1024] = {0};
    while(isMainMenuRunning){
        cin.getline(buffer,1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if(-1 == idx){
            command = commandbuf;
        }else{
            command = commandbuf.substr(0,idx);
        }
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end()){                  //没找到命令号
            cerr << "invalid input command." << endl;
            continue;
        }
        //调用相应的命令处理回调。
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));
    }   
}

int main(int argc,char** argv){
    if(argc < 3){
        cerr << "Error Input." << endl;
    }
    //创建socket
    int cliSocket = socket(AF_INET,SOCK_STREAM,0);
    if(cliSocket == -1){
        cerr << "create client socket error." << endl;
        exit(-1);
    }
    //写入服务器地址
    sockaddr_in Servaddr;
    memset(&Servaddr,0,sizeof(sockaddr_in));
    Servaddr.sin_family = AF_INET;
    Servaddr.sin_port = htons(atoi(argv[2]));
    Servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    //连接
    if(-1 == connect(cliSocket,(sockaddr*)&Servaddr,sizeof(sockaddr_in))){
        cerr << "connect server error." << endl;
        close(cliSocket);
        exit(0);
    }
    //main线程:接收用户输入,发送数据。
    for(;;){
        cout << "===============================" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;
        cout << "===============================" << endl;
        cout << "choice:" ;
        int choice = 0;
        cin >> choice;
        cin.get();
        switch(choice){
            case 1:         //登录
            {
                cout << "Please Input your userid:" ;
                string id;
                getline(cin,id);
                cout << "Please Input your password:";
                string password;
                getline(cin,password);
                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = stoi(id);
                js["password"] = password;
                string request = js.dump();
                int len = send(cliSocket,request.c_str(),strlen(request.c_str())+1,0);
                if(-1 == len){
                    cerr << "send reg msg error:" << request << endl;
                }
                else{
                    char buffer[1024] = {0};
                    len = recv(cliSocket,buffer,1024,0);
                    if(-1 == len){
                        cerr << "recv reg response error." << endl;
                    }else{
                        json responsejs = json::parse(buffer);
                        //cout << responsejs.dump() << endl;
                        if(0 == responsejs["errno"].get<int>()){
                            string name = responsejs["name"];
                            cerr << name << " login successfully." << endl;
                            //登录成功-------登记当前系统登录的用户信息、好友列表、群组列表。
                            _currentUser.setId(responsejs["id"].get<int>());
                            _currentUser.setName(responsejs["name"]);
                            //好友列表信息
                            if(responsejs.contains("friends")){
                                //初始化
                                _currentUserFriendsList.clear();
                                vector<string> vec = responsejs["friends"];
                                for(string &str:vec){
                                    json js = json::parse(str);
                                    User user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    _currentUserFriendsList.push_back(user);
                                }
                            }
                            //群组列表信息
                            //用户的群组信息:groups:id-groupname-groupdesc-users
                            if(responsejs.contains("groups")){
                                vector<string> vec1 = responsejs["groups"];
                                _currentUserGroupsList.clear();
                                for(string &groupStr:vec1){
                                    json grpjson = json::parse(groupStr);
                                    AllGroup group;
                                    group.setId(grpjson["id"].get<int>());
                                    group.setGroupname(grpjson["groupname"]);
                                    group.setGroupdesc(grpjson["groupdesc"]);

                                    vector<string> userV = grpjson["users"];
                                    for(string &userStr:userV){
                                        json js = json::parse(userStr);
                                        GroupUser gpuser;
                                        gpuser.setId(js["id"].get<int>());
                                        gpuser.setName(js["name"]);
                                        gpuser.setState(js["state"]);
                                        gpuser.setGrouprole(js["grouprole"]);
                                        group.getUsers().push_back(gpuser);
                                    }
                                    _currentUserGroupsList.push_back(group);
                                }
                            }
                            ShowCurrentUserData();
                            //显示离线消息 个人/群组离线消息。
                            if(responsejs.contains("offlinemsg")){
                                cout << "-----------------History Message-------------------" << endl;
                                vector<string> vec = responsejs["offlinemsg"];
                                for(string &str:vec){
                                    json js = json::parse(str);
                                    cout << js["time"] << ":[" << js["userid"] << "]" << js["name"]
                                         << " said: " << js["msg"] << endl;
                                }
                            }
                            static int threadNum = 0;
                            if(threadNum == 0){
                                //登录成功，启动接收线程接收数据。只开启一次 ~ pthread_create
                                std::thread readTask(readTaskHandler,cliSocket);
                                //分离线程 ~ pthread_detach. 分离线程。自动回收。
                                readTask.detach();
                                threadNum++;
                            }
                            
                            //进入聊天主菜单
                            isMainMenuRunning = true;
                            mainMenu(cliSocket);
                        }else{
                            //登录失败-------打印失败信息。
                            cout << js["errmsg"] << endl;
                        }
                    }                    
                }
                break;
            }
            case 2:         //注册
            {
                cout << "Please Input your name:" ;
                string name;
                getline(cin,name);
                cout << "Please Input your Password:";
                string password;
                getline(cin,password);
                //注册-->msgid:REF
                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;                      //检验消息。=0无误
                js["password"] = password;
                string request = js.dump();
                int len = send(cliSocket,request.c_str(),strlen(request.c_str())+1,0);
                if(len == -1){
                    //cout << "send error." << endl;
                    cerr << "send reg msg error:" << request << endl;
                }else{
                    char buffer[1024] = {0};
                    len = recv(cliSocket,buffer,1024,0);
                    if(-1 == len){
                        cerr << "recv reg response error." << endl;
                    }else{
                        json responsejs = json::parse(buffer);
                        if(0 != responsejs["errno"].get<int>()){
                            cerr << name << " is already exist,register error." << endl;
                        }else{
                            cout << name << " register success.Your userid is " << responsejs["id"] << endl;
                        }
                    }
                }
                break;
            }
            case 3:         //quit
            {
                close(cliSocket);
                exit(0);
                break;
            }
            default:
            {
                cerr << "Invalid input." << endl;
                break;
            }
        }
    }
    return 0;

}


void help(int,string){
    cout << "------------show command list------------" << endl;
    for(auto &p:commandMap){
        cout << p.first << ":" << p.second << endl;
    }
    cout << "-----------------------------------------" << endl;
}
void chat(int clientfd,string message){
    int idx = message.find(":");
    if(-1 == idx){
        cerr << "chat command invalid." << endl;
        return ;
    }
    int friendid = atoi(message.substr(0,idx).c_str());         //message= friendid:msg
    string msg = message.substr(idx+1,message.size()-idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = _currentUser.getId();
    js["name"] = _currentUser.getName();
    js["dstid"] = friendid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

void addfriend(int clientfd,string msg){
    int friendid =  atoi(msg.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = _currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}
void creategroup(int clientfd,string message){
    int idx = message.find(":");
    if(idx == -1){
        cerr << "creategroup command invalid." << endl;
        return ; 
    }
    string groupname = message.substr(0,idx);
    string groupdesc = message.substr(idx+1,message.size()-idx);
    json js;
    js["msgid"] = CTE_GROUP_MSG;
    js["userid"] = _currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr << "send creategroup msg error -> " << buffer << endl;
    }    
}
void addgroup(int clientfd,string message){
    int groupid = atoi(message.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["userid"] = _currentUser.getId();
    js["groupid"] = groupid;

    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr << "send addgroup msg error -> " << buffer << endl;
    }    
}
void groupchat(int clientfd,string message){
    int idx = message.find(":");
    if(idx == -1){
        cerr << "groupchat command error." << endl;
        return;
    }
    int groupid = atoi(message.substr(0,idx).c_str());
    string msg = message.substr(idx+1,msg.size()-idx);    
    json js;
    js["msgid"] = GRP_CHAT_MSG;
    js["userid"] = _currentUser.getId();
    js["name"] = _currentUser.getName();
    js["time"] = getCurrentTime();
    js["groupid"] = groupid;
    js["msg"] = msg;

    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr << "send addgroup msg error -> " << buffer << endl;
    }    
}
void loginout(int clientfd,string str){
    json js;
    js["msgid"] = LOGIN_OUT;
    js["id"] = _currentUser.getId();
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len){
        cerr << "send loginout message error." << endl;
    }else{
        isMainMenuRunning = false;
    }
}
