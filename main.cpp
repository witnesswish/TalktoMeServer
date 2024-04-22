#include "Server.h"
#include <string.h>
#include <iostream>
#include <regex>
#include <vector>
#include <ctype.h>
#include <stdlib.h>
#include <openssl/crypto.h>

void tagmain(int argc, char *argv[])
{
    for(int i=0; i<argc; ++i)
    {
        std::cout<< argv[i] <<std::endl;
    }
}

/** isport
return
0 for is legel port, bettwen 100-5656
-1 for not a legel port
**/
int isport(char *argv1)
{
    return 0;
}

int main(int argc, char *argv[])
{
    std::regex reg_ipv4("(((2(5[0-5]|[0-4])\\d)|([0-1]?\\d{1,2}))\\.){3}(2(5[0-5]|[0-4])\\d|[0-1]?\\d{1,2})$");
    std::regex reg_port("\\d{3,4}");
    if(argc == 1)
    {
        Server serv;
        serv.StartServer();
        return 0;
    }
    if(argc >= 2)
    {
        std::vector<std::string> options;
        for(int i=1; i<argc; ++i)
        {
            options.push_back(argv[i]);
            std::string sig_option = options[i].substr(0, 1);
            if(sig_option == "-")
            {
                sig_option = options[i].substr(1,2);
                if(sig_option == "-")
                {
                    if(strncasecmp(argv[i], "host", strlen("host")) == 0)
                    {
                        Server serv(argv[i], 3434);
                        serv.StartServer();
                        return 0;
                    }
                    if(strncasecmp(argv[i], "port", strlen("port")) == 0)
                    {
                        int port = atoi(argv[i]);
                        Server serv("127.0.0.1", port);
                        serv.StartServer();
                        return 0;
                    }
                    if(strncasecmp(argv[i], "help", strlen("help")) == 0)
                    {
                        std::cout<< "Usage: \n -h --host for host, basically is 127.0.0.1. \n -p --port for port, basically is 3434." <<std::endl;
                        return 0;
                    }
                }
            }
        }
        if(strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            std::cout<< "Usage: \n -h --host for host, basically is 127.0.0.1. \n -p --port for port, basically is 3434." <<std::endl;
            return 0;
        }

        if(0)
        {
            if(std::regex_match(argv[1], reg_port))
            {
                tagmain(argc, argv);
                return 0;
            }
            else
            {
                std::cout<< "Looks like you want to input the port, but you make the mistake, just fix it and try again." <<std::endl;
                return 0;

            }
        }
        if(std::regex_match(argv[1], reg_ipv4))
        {
            tagmain(argc, argv);
            return 0;
        }
        else
        {
            std::cout<< "Looks like you want to input the ipv4 address, but you make the mistake, just fix it and try again." <<std::endl;
            return 0;
        }
    }
}
