make: g++ -o main *.cpp *.h -lsqlite3 -lmysqlclient
    You may installed the mysql(or mariadb) devel kit, but still got the compile error like this: cannot find -lmysqlclient, you should try `mysql_config` to get where your lib is, and your compile command would looks like this: `g++ -o main *.cpp *.h -lsqlite3 -L/usr/lib64/mysql -lmysqlclient -lpthread `
