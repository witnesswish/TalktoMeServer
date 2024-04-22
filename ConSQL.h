#ifndef TALKTOME_CONSQL_H
#define TALKTOME_CONSQL_H

#include <mysql/mysql.h>
#include <cstddef>
#include <stdio.h>
#include <iostream>
#include <string.h>

class ConSQL
{
public:
    ConSQL();
    ~ConSQL();
public:
    int Mconnect(char*);
    void Mdisconnect();
    /** function Mlogin: 0 for success -1 for failure **/
    int Mlogin(const char*, const char*, char *, char *);
private:
    MYSQL *mysql;
    MYSQL_RES *Result;
    MYSQL_ROW Row;
    MYSQL_FIELD *Field;

};

#endif // TALKTOME_CONSQL_H
