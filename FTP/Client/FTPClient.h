#pragma once

#include<string>
class FTPClient
{
public:
    bool Connect(const std::string& ip,int port);
    void Close();
	FTPClient();
	~FTPClient();
    bool Login(const std::string& user, const std::string& pass);
    void DoList();
    void DoGet(const std::string& filename);    
    void DoPut(const std::string& filename);   
    void Quit(); 
private:
    int _ControlFd;
    std::string SendCmd(const std::string& command);
    bool ParsePasv(const std::string& response,std::string& ip,int& port);
    int DataConnect(const std::string& ip,int port);
    std::string RecvResponse();

    
};
