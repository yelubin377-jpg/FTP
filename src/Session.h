#pragma once //只使用一次
#include<string>

class Session
{
public:
    Session(int ClientFd,const std::string& RootDir);  //Session 会话，命令分析 构造函数
    ~Session(); 
    void run(); 
private:
    int _ClientFd; //文件提示符
    std::string _RootDir; //根目录
    std::string _Username; //用户名
    bool _WhetherLogin; //是否登录，登录锁
    int _DataFd; //数据文件提示符
};