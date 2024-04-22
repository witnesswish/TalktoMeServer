#ifndef TALKTOME_ONLINEHANDLER_H
#define TALKTOME_ONLINEHANDLER_H

#include <list>
#include <iostream>

#include "ConSQL.h"
#include "Common.h"

struct User
{
    int Fd;
    char UserName[14];
};
struct GroupUser
{
    char GroupName[255];
};
struct Group
{
    char GroupName[255];
    User u;
};

class Server;

class OnlineHandler
{
    friend Server;
public:
    OnlineHandler();
    ~OnlineHandler();
public:
    int FindUser(const char*);                //return user fd and -1 for error occured.
    int AddNewUser(const char*, int);         //new user login, add new user to online_list, 0 for success, -1 for error
    int RemoveUser(int);                //user logout, remove user from online_list, 0 for success, -1 for error
    int Heartbeat();

private:
    ConSQL *mysql;
    std::list<Group> *OnlineGroupUser;
    std::list<User> *OnlineUser;
};

#endif // TALKTOME_ONLINEHANDLER_H
