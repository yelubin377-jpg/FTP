#pragma once
#include <string> 
#include <atomic>


class Server
{
public:
    explicit Server(int port,std::string RootDir);
    ~Server();
    void run();
    void stop();
private:
    int _ListenFd;
    int _Port;
    std::string _RootDir;
    std::atomic<bool> _Running{false};


};
