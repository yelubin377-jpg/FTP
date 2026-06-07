#include "FTPClient.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
int main()
{
    FTPClient client;
    if(!client.Connect("127.0.0.1",2100))
    {
        std::cerr << "main:connect boom()!!!" << std::endl;
        return 1;
    }
    std::string user , pass;
    std::cout << "用户名:" << std::endl;
    std::cin >> user;
    std::cout << "密码:" << std::endl;
    std::cin >> pass;
    if(!client.Login(user, pass)) //
    {
        std::cerr << "main:login boom()!!!" << std::endl;
        return 1;
    }
    
    //

    std::string cmd;
    while(std::cin >> cmd)
    {
        if(cmd == "ls")   client.DoList(); //
        else if(cmd == "get")
        {
            std::string f;
            std::cin >> f;
            client.DoGet(f);
        }
        else if(cmd == "put")
        {
            std::string f;
            std::cin >> f;
            client.DoPut(f);
        }
        else if(cmd == "quit")
        {
            break;
        }
    }
    client.Quit();
    client.Close();
    return 0;


}