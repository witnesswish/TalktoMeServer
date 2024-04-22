#include "ConSQL.h"

ConSQL::ConSQL()
{

}
ConSQL::~ConSQL() {}

int ConSQL::Mconnect(char *Sqlname)
{
    mysql = mysql_init(NULL);
    mysql = mysql_real_connect(mysql, "127.0.0.1", "root", "`12", Sqlname, 3306, NULL, 0);
    if(mysql == NULL)
    {
        perror("connect to msyql error: ");
        return -1;
    }
    if(mysql_query(mysql, "set names utf8;") != 0)
    {
        std::cout<< "mysql query error" <<std::endl;
        return -1;
    }
    return 0;
}
void ConSQL::Mdisconnect()
{
    mysql_close(mysql);
}

int ConSQL::Mlogin(const char *username, const char *password, char *friends, char *groups)
{
    char sql[280] = {0};
    sprintf(sql, "select tuser.Auth_string, tuserinfo.friends, tuserinfo.groups from tuser, tuserinfo where tuser.id=tuserinfo.uid and tuser.User='%s';", username);
    if(mysql_query(mysql, sql))
    {
        perror("mysql login query error: ");
        return -1;
    }
    else
    {
        Result = mysql_store_result(mysql);
        //int FieldNum = mysql_num_fields(Result);
        /** actually RowNum is no needed, the result will be only one or zero **/
        int RowNum = mysql_num_rows(Result);
        for(int i=0; i<RowNum; ++i)
        {
            Row = mysql_fetch_row(Result);
            if(Row == NULL)
            {
                perror("mysql login fetch row error: ");
                mysql_free_result(Result);
                return -1;
            }
            else if(std::string(Row[0]) == std::string(password))
            {
                strcpy(friends, Row[1]);
                strcpy(groups, Row[2]);
                mysql_free_result(Result);
                return 0;
            }
            else
            {
                mysql_free_result(Result);
                return -1;
            }
        }
        mysql_free_result(Result);
        return -1;
    }
}
