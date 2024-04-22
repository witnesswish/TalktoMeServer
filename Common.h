#ifndef TALKTOME_COMMON_H
#define TALKTOME_COMMON_H

#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>

#define WELCOME_MESSAGE "You are connected to TalktoMe server, welcome."

static void epoll_add_fd(int fd, int epfd, bool et_enable)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if(et_enable)
    {
        ev.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

static int tag_count = 0;

template <typename T>
void tag(T t)
{
    tag_count += 1;
    std::cout<< "----------tag here ------- " << "tag num is: " << tag_count << ": " << t <<std::endl;
}
template <typename T1, typename T2>
void tag(T1 t, T2 &arg1)
{
    tag_count += 1;
    std::cout<< "----------tag here ------- " << "tag num is: " << tag_count << ": " << t << " arg1: " << arg1 <<std::endl;
}
template <typename T3, typename T4, typename T5>
void tag(T3 t, T4 arg1, T5 arg2)
{
    tag_count += 1;
    std::cout<< "----------tag here ------- " << "tag num is: " << tag_count << ": " << t << " arg1: " << arg1 << " arg2: " << arg2 <<std::endl;
}

template <typename T>
void ServerMessage(T t)
{
    std::cout<< "---Server message: " << t <<std::endl;
}
template <typename T1, typename T2, typename T3>
void ServerMessage(T1 t, T2 arg1, T3 arg2)
{
    std::cout<< "Server message: " << t << " " << arg1 << " " << arg2 <<std::endl;
}

struct Msg
{
    int FromId;             //for test when it doesn't link with sql, or not got a name
    int ToId;
    char ClientType[11];    //windows linux android commandline(only linux)
    char Username[14];
    char Password[14];
    char CmdType[21];           //server_reply client_request client_message etc
    char Cmd[21];           //login login_reply etc
    int result;             //0 for success, -1 for failure, others forbidden
    char ToUser[14];
    char FromUser[14];
    char UserGroups[255];
    char UserFriends[255];
    char Groupnames[255];
    char Content[1024];
};

#endif // TALKTOME_COMMON_H
