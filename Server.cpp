#include "Server.h"

sqlite3 *Server::lite3 = NULL;
OnlineHandler *Server::OnlineHelper = new OnlineHandler;
std::list<int> Server::online_user = {0};
Server::Server(const char *ip, int port)
{
    master = 0;
    epfd = 0;
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = inet_addr(ip);
    ServAddr.sin_port = htons(port);
    sql = new ConSQL;
}
Server::~Server()
{
    close(master);
    close(epfd);
}

void Server::Initializer()
{
    master = socket(AF_INET, SOCK_STREAM, 0);
    if(master < 0)
    {
        perror("socket create error: ");
        return;
    }
    int opt = 1;
    setsockopt(master, SOL_SOCKET, SO_REUSEADDR, &opt, (socklen_t)sizeof(opt));
    if(bind(master, (const struct sockaddr *)&ServAddr, sizeof(ServAddr)) < 0)
    {
        perror("bind socket error: ");
        return;
    }
    if(listen(master, 10) < 0)
    {
        perror("listen socket error: ");
        return;
    }
    epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0)
    {
        perror("epoll create error: ");
        return;
    }
    epoll_add_fd(master, epfd, true);
    if(sql->Mconnect((char*)"talktome"))
    {
        perror("mysql init error: ");
        return;
    }
    if(sqlite3_open("talktome.db", &lite3) != SQLITE_OK)
    {
        perror("open talktome.db error: ");
        return;
    }
    char *EEr = NULL;
    if(sqlite3_exec(lite3, "create TABLE if not exists main.user(fd int primary key not null, name text not null, states int default 0, updated_time text not null);", NULL, NULL, &EEr) != SQLITE_OK)
    {
        perror("sqlite table create check error: ");
    }
    if(sqlite3_exec(lite3, "delete from user;", NULL, NULL, &EEr) != SQLITE_OK)
    {
        perror("sqlite table create check error: ");
    }
    /*if(sqlite3_close(lite3) != SQLITE_OK)
    {
        perror("close talktome.db error: ");
        return;
    }*/
    std::cout<< "server started" <<std::endl;
}

/**
* states int
*
**/
void Server::Heartbeat(sqlite3 *lite3)
{
    time_t past = time(NULL);
    while(1)
    {
        sleep(0);
        time_t now = time(NULL);
        char tmpnow[32];
        char tmppast[32];
        sprintf(tmppast, "%ld", past);
        sprintf(tmpnow, "%ld", now);
        char *er = 0;
        if(atoi(tmpnow)-atoi(tmppast)>6)
        {
            if(sqlite3_exec(lite3, "select * from user;", &HtHandler, 0, &er) != SQLITE_OK)
            {
                std::cout<< "er: "<< er <<std::endl;
                perror("handled----------- sql state error");
            }
            past = now;
        }
    }
    /**time_t start_t, end_t;
    double diff_t;
    start_t = 0;
    while(1)
    {

        time(&end_t);
        diff_t = difftime(end_t, start_t);
        if(diff_t>double(60))
        {
            for(std::list<int>::iterator it=online_user.begin(); it != online_user.end(); ++it)
            {

                memset(&msg, 0, sizeof(msg));
                strcpy(msg.CmdType, "heartbeat");
                strcpy(msg.Cmd, "heartbeat_check");
                bzero(SendBuffer, sizeof(SendBuffer));
                memcpy(SendBuffer, &msg, sizeof(SendBuffer));
                if(send(*it, SendBuffer, sizeof(SendBuffer), 0) < 0)
                {
                    perror("send heartbeat error: ");
                }
            }
            start_t = end_t;
        }
    }**/
}

void Server::StartServer()
{
    Server serv;
    struct epoll_event events[EPOLL_SIZE];
    Initializer();
    std::thread THeartbeat(&Server::Heartbeat, &serv, lite3);
    THeartbeat.detach();
    while(1)
    {
        int epoll_event_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if(epoll_event_count < 0)
        {
            perror("epoll event wait error: ");
            return;
        }
        for(int i=0; i<epoll_event_count; ++i)
        {
            int fd = events[i].data.fd;
            /** new client in
                send client welcome message and line it up
            **/
            if(fd == master)
            {
                struct sockaddr_in ClientAddr;
                socklen_t ClientAddrLen = sizeof(struct sockaddr);
                int ClientFd = accept(master, (struct sockaddr *)&ClientAddr, &ClientAddrLen);
                std::cout<< "Client #" << ClientFd << " connect from " << inet_ntoa(ClientAddr.sin_addr) << ":" << ntohs(ClientAddr.sin_port) <<std::endl;
                memset(&msg, 0, sizeof(msg));
                strcpy(msg.Content, WELCOME_MESSAGE);
                strcpy(msg.CmdType, "welcome_message");
                strcpy(msg.Cmd, "welcome");
                memcpy(SendBuffer, &msg, sizeof(msg));
                if(send(ClientFd, SendBuffer, sizeof(SendBuffer), 0) < 0)
                {
                    perror("send welcome message error: ");
                }
                else
                {
                    epoll_add_fd(ClientFd, epfd, true);
                    online_user.push_back(ClientFd);
                }
            }
            else
            {
                int ret = read_cb(fd);
                if(ret < 0)
                {
                    perror("read_cb error: ");
                }
            }
        }
    }
}

int Server::read_cb(int fd)
{
    std::cout<< "read form client #" << fd <<std::endl;
    bzero(RecvBuffer, sizeof(RecvBuffer));
    memset(&msg, 0, sizeof(msg));
    Msg tmpmsg;
    memset(&tmpmsg, 0, sizeof(tmpmsg));
    int len = recv(fd, RecvBuffer, sizeof(RecvBuffer), 0);
    //tag(len);
    if(len < 0)
    {
        perror("recv from client error: ");
        return -1;
    }
    if(len == 0)
    {
        for(auto it=online_user.begin(); it != online_user.end();)
        {
            if(*it == fd)
            {
                online_user.erase(it++);
            }
            else
                it++;
        }
        int ret = OnlineHelper->RemoveUser(fd);
        //tag("remove ret", ret);
        if(ret < 0)
        {
            return -1;
        }
        else
        {
            ServerMessage("client #", fd, "closed connection.");
            return 0;
        }
    }
    else
    {
        memcpy(&tmpmsg, RecvBuffer, sizeof(RecvBuffer));
        std::string Cmd_Type = tmpmsg.CmdType;
        std::string Cmd = tmpmsg.Cmd;
        std::string username = (tmpmsg.Username);
        std::string password = (tmpmsg.Password);
        //tag(tmpmsg.FromUser, tmpmsg.ToUser, tmpmsg.Content);
        if(Cmd_Type == "client_login_request")
        {
            memcpy(&msg, &tmpmsg, sizeof(msg));
            if(strncasecmp(msg.Cmd, "login", strlen("login")) == 0)
            {
                int isLogin = OnlineHelper->FindUser(username.c_str());
                //tag("isLogin", isLogin);
                if(isLogin > 0)
                {
                    //tag("isLogin is greater than 1", isLogin);
                    Msg tmsg;
                    memset(&tmsg, 0, sizeof(tmsg));
                    strcpy(tmsg.CmdType, "server_login_reply");
                    strcpy(tmsg.Cmd, "login_reply");
                    tmsg.result = -3;
                    strcpy(tmsg.Content, "login failure, multiple login is not allowed.");
                    memcpy(tmsg.Username, msg.Username, sizeof(msg.Username));
                    bzero(SendBuffer, sizeof(SendBuffer));
                    memcpy(SendBuffer, &tmsg, sizeof(tmsg));
                    int ret = send(fd, SendBuffer, sizeof(SendBuffer), 0);
                    if(ret < 0)
                    {
                        perror("send login_reply error: ");
                        return -1;
                    }
                    else
                    {
                        return len;
                    }
                }
                sql->Mconnect((char*)"talktome");
                int LoginStatus = sql->Mlogin(username.c_str(), password.c_str(), msg.UserFriends, msg.UserGroups);
                if(LoginStatus == 0)
                {
                    bzero(SqlMsg, sizeof(SqlMsg));
                    time_t now = time(NULL);
                    char tmp[32];
                    sprintf(tmp, "%ld", now);
                    sprintf(SqlMsg, "insert into user(fd, name, states, updated_time) values (%d, '%s', 0, '%s');", fd, username.c_str(), tmp);
                    if(sqlite3_exec(lite3, SqlMsg, NULL, NULL, 0) != SQLITE_OK)
                    {
                        perror("insert new user to sqlite error: ");
                    }
                    int ret_addonlist = OnlineHelper->AddNewUser(username.c_str(), fd);
                    //tag("ret_addonlist", ret_addonlist);
                    if(ret_addonlist < 0)
                    {
                        perror("add to online user list error");
                    }
                    strcpy(msg.CmdType, "server_login_reply");
                    strcpy(msg.Cmd, "login_reply");
                    msg.result = 3;
                    strcpy(msg.Content, "login success");
                    //memcpy(msg.UserFriends, msg.UserFriends, sizeof(msg.UserFriends));
                    //memcpy(msg.UserGroups, msg.UserGroups, sizeof(msg.UserGroups));
                    bzero(SendBuffer, sizeof(SendBuffer));
                    memcpy(SendBuffer, &msg, sizeof(msg));
                    int ret = send(fd, SendBuffer, sizeof(SendBuffer), 0);
                    if(ret < 0)
                    {
                        perror("send login_reply error: ");
                        return -1;
                    }
                    else
                    {
                        return len;
                    }
                }
                else
                {
                    Msg tmsg;
                    memset(&tmsg, 0, sizeof(tmsg));
                    strcpy(tmsg.CmdType, "server_login_reply");
                    strcpy(tmsg.Cmd, "login_reply");
                    tmsg.result = -1;
                    strcpy(tmsg.Content, "login failure");
                    memcpy(tmsg.Username, msg.Username, sizeof(msg.Username));
                    bzero(SendBuffer, sizeof(SendBuffer));
                    memcpy(SendBuffer, &tmsg, sizeof(tmsg));
                    int ret = send(fd, SendBuffer, sizeof(SendBuffer), 0);
                    if(ret < 0)
                    {
                        perror("send login_reply error: ");
                        return -1;
                    }
                    else
                    {
                        return len;
                    }
                }
            }
        }
        if(Cmd_Type == "private")
        {
            memcpy(&msg, &tmpmsg, sizeof(msg));
            if(Cmd == "private_message")
            {
                int ToId = OnlineHelper->FindUser(msg.ToUser);
                tag("private find user res: ", ToId);
                int FromId = fd;
                if(ToId < 0)
                {
                    strcpy(msg.CmdType, "private");
                    strcpy(msg.Cmd, "private_reply");
                    msg.result = -1;
                    strcpy(msg.Content, "user is not online, system will not keep message for you, just try another time.");
                    //memcpy(msg.Username, msg.Username, strlen(msg.Username));
                    bzero(SendBuffer, sizeof(SendBuffer));
                    memcpy(SendBuffer, &msg, sizeof(SendBuffer));
                    if(send(fd, SendBuffer, sizeof(SendBuffer), 0) < 0)
                    {
                        perror("send private_reply error: ");
                        return -1;
                    }
                    else
                    {
                        return len;
                    }
                }
                else
                {
                    msg.FromId = FromId;
                    strcpy(msg.CmdType, "private");
                    strcpy(msg.Cmd, "private_boradcast");
                    bzero(SendBuffer, sizeof(SendBuffer));
                    memcpy(SendBuffer, &msg, sizeof(msg));
                    int ret = send(ToId, SendBuffer, sizeof(SendBuffer), 0);
                    if(ret < 0)
                    {
                        perror("send private_boradcast error: ");
                        return -1;
                    }
                    else
                    {
                        return len;
                    }
                }
            }
        }
        if(Cmd_Type == "heartbeat")
        {
            if(Cmd == "heartbeat_reply")
            {
                /** SqliteResults for result of sqlite that profit **/
                std::vector<int> SqliteResults;
                bzero(SqlMsg, sizeof(SqlMsg));
                time_t now = time(NULL);
                char tmp[32];
                char *errmsg;
                sprintf(tmp, "%ld", now);
                sprintf(SqlMsg, "update user set updated_time='%s', states=0 where fd=%d;", tmp, fd);
                if(sqlite3_exec(lite3, SqlMsg, NULL, NULL, 0) != SQLITE_OK)
                {
                    perror("update user state to sqlite error: ");
                }
                sprintf(SqlMsg, "select * from user;");
                if(sqlite3_exec(lite3, SqlMsg, &HeartbeatHandler, &SqliteResults, &errmsg) != SQLITE_OK)
                {
                    perror("handle sql state error: ");
                    tag(errmsg);
                }
                else
                {
                    for(auto it=SqliteResults.begin(); it != SqliteResults.end(); ++it)
                    {
                        tag("I am gonna to remove it", *it);
                        if(OnlineHelper->RemoveUser(*it) < 0)
                        {
                            perror("ht reply remove fd error");
                        }
                        online_user.remove(*it);
                        close(*it);
                        sprintf(SqlMsg, "delete from user where fd=%d;", *it);
                        if(sqlite3_exec(lite3, SqlMsg, NULL, NULL, &errmsg) != SQLITE_OK)
                        {
                            perror("handle sql state error: ");
                            tag(errmsg);
                        }
                    }
                }
            }
        }
        return len;
    }
    return len;
}

int Server::HeartbeatHandler(void *data, int nColumn, char **colValues, char **colNames)
{
    std::vector<int> *res = (std::vector<int> *)data;
    char sql_tmp[1024];
    bzero(sql_tmp, sizeof(sql_tmp));
    time_t now = time(NULL);
    char tmp[32];
    sprintf(tmp, "%ld", now);
    if(atoi(tmp)-atoi(colValues[3]) > 5)
    {
        sprintf(sql_tmp, "update user set states=states+1 where name='%s';", colValues[1]);
        if(sqlite3_exec(lite3, sql_tmp, NULL, NULL, 0) != SQLITE_OK)
        {
            perror("add user heartbeat state error: ");
            return -1;
        }
    }
    if(atoi(colValues[2]) > 5)
    {
        int n = atoi(colValues[0]);
        res->push_back(n);
    }
    return 0;
}
int Server::HtHandler(void *data, int nColumn, char **colValues, char **colNames)
{
    Msg tmsg;
    char sql_tmp[1024];
    bzero(sql_tmp, sizeof(sql_tmp));
    char SendBuffer[sizeof(Msg)];
    memset(&tmsg, 0, sizeof(tmsg));
    strcpy(tmsg.CmdType, "heartbeat");
    strcpy(tmsg.Cmd, "heartbeat_check");
    bzero(SendBuffer, sizeof(SendBuffer));
    memcpy(SendBuffer, &tmsg, sizeof(SendBuffer));
    char *er = 0;
    if(send(atoi(colValues[0]), SendBuffer, sizeof(SendBuffer), 0) < 0)
    {
        sprintf(sql_tmp, "update user set states=states+1 where name='%s';", colValues[1]);
        if(sqlite3_exec(lite3, sql_tmp, NULL, NULL, &er) != SQLITE_OK)
        {
            perror("change user heartbeat state error: ");
            tag(er);
            return -1;
        }
    }
    return 0;
}
