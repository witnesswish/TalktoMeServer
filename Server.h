#ifndef TALKTOME_SERVER_H
#define TALKTOME_SERVER_H

#include <list>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <thread>
#include <vector>

#include "Common.h"
#include "ConSQL.h"
#include "OnlineHandler.h"

#define EPOLL_SIZE 5000

struct LiveStatus
{
    int Fd;
    int StatusCount;
};

class Server
{
public:
    Server(const char *ip="0.0.0.0", int port=3434);
    ~Server();
    void Initializer();
    void StartServer();
    void CloseServer();
    int read_cb(int fd);
    void Heartbeat(sqlite3 *lite3);
    static int HtHandler(void *data, int nColumn, char **colValues, char **colNames);
    static int HeartbeatHandler(void *data, int nColumn, char **colValues, char **colNames);
private:
    Msg msg;
    int master;
    int epfd;
    static sqlite3 *lite3;
    char SqlMsg[1024];
    static std::list<int> online_user;
    struct sockaddr_in ServAddr;
    char RecvBuffer[sizeof(Msg)];
    char SendBuffer[sizeof(Msg)];
private:
    ConSQL *sql;
    static OnlineHandler *OnlineHelper;
};

#endif // TALKTOME_SERVER_H
