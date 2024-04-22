#include "OnlineHandler.h"

OnlineHandler::OnlineHandler()
{
    mysql = new ConSQL;
    OnlineGroupUser = new std::list<Group>;
    OnlineUser = new std::list<User>;
};
OnlineHandler::~OnlineHandler() {};

int OnlineHandler::FindUser(const char *name)
{
    for(std::list<User>::iterator it=OnlineUser->begin(); it!=OnlineUser->end(); ++it)
    {
        std::string UserName(name);
        std::string Online_Name = it->UserName;
        if(Online_Name == UserName)
        {
            return it->Fd;
        }
    }
    return -1;
}
int OnlineHandler::RemoveUser(int Fd)
{
    ServerMessage("removing user fd #", Fd, Fd);
    if(OnlineUser->size() == 0)
    {
        return 0;
    }
    for(std::list<User>::iterator it=OnlineUser->begin(); it != OnlineUser->end();)
    {
        if(it->Fd == Fd)
        {
            OnlineUser->erase(it++);
            return 0;
        }
        else
        {
            it++;
        }
    }
    /** user connected but not login **/
    for(std::list<User>::iterator it=OnlineUser->begin(); it != OnlineUser->end(); ++it)
    {
        if(it->Fd == Fd)
        {
            return -1;
        }
    }
    return 0;
}
int OnlineHandler::AddNewUser(const char *UserName, int Fd)
{
    User u;
    for(std::list<User>::iterator it=OnlineUser->begin(); it != OnlineUser->end(); ++it)
    {
        if(it->Fd == Fd)
        {
            return -1;
        }
    }
    u.Fd = Fd;
    memcpy(u.UserName, UserName, sizeof(u.UserName));
    ServerMessage("addnewuser.username", u.UserName, u.UserName);
    OnlineUser->push_back(u);
    return 0;
}

/**
* return
*
**/
int OnlineHandler::Heartbeat()
{
    return 0;
}
