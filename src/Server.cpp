#include "Server.h"
#include "Session.h"

#include<cstdio>
#include<iostream>
#include<cstring>
#include<thread>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
//
Server::Server(int port,std::string RootDir)
    :_Port(port)
    ,_RootDir(std::move(RootDir))
{
}
Server::~Server()
{
}
//socket 到  
void Server::run()
{
    _ListenFd=socket(AF_INET,SOCK_STREAM,0);
    if(_ListenFd<0)
    {
        std::cerr << "socket() boom!!!:" << std::strerror(errno) << std::endl;
        return;
    }

    //

    int opt = 1;
    setsockopt(_ListenFd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    //

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_Port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //

    int BindRet = bind(_ListenFd,(sockaddr*)&addr,sizeof(addr));                                                          
    if(BindRet<0)
    {
        std::cerr << "bind() boom!!!:" << std::strerror(errno) << std::endl;
        return;
    }

    //

    int ListenRet = listen(_ListenFd,SOMAXCONN);
    if(ListenRet<0)
    {
        std::cerr << "listen() boom!!!" << std::strerror(errno) << std::endl;
        return;
    }

    //

    printf("Server listening on port %d\n",_Port);

    //

    _Running = true;
    while(_Running)
    {
        struct sockaddr_in ClientAddr;
        socklen_t ClientLen = sizeof(ClientAddr);
        int AcceptRet = accept(_ListenFd,(sockaddr*)&ClientAddr,&ClientLen);  //AcceptRet其实就是ClientFd
        if(AcceptRet<0)
        {
            continue;
        }
        printf("Clients' IP is:%s\n",inet_ntoa(ClientAddr.sin_addr));

        //

        std::thread([this,AcceptRet]()
        {
            Session session(AcceptRet,_RootDir);
            session.run();
        }).detach();
    }   

    //







    

}


void Server::stop()
{
    _Running = false;
    close(_ListenFd);
}