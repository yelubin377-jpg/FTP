#include "FTPClient.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>


FTPClient::FTPClient()
    :_ControlFd(-1)
{
}

FTPClient::~FTPClient()
{
    Close();
}


bool FTPClient::Connect(const std::string& ip,int port)    
{   
    _ControlFd = socket(AF_INET,SOCK_STREAM,0);
    if(_ControlFd < 0)
    {
        std::cerr << "socket error" << std::endl;
        return false; //
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET,ip.c_str(),&addr.sin_addr);
    if(connect(_ControlFd,(sockaddr*)&addr,sizeof(addr))<0)
    {
        std::cerr << "connect boom()!!!" << std::endl;
        close(_ControlFd);
        return false;
    }
    std::string welcome = RecvResponse(); //
    std::cout << welcome << std::endl; //
    return true;
}



std::string FTPClient::SendCmd(const std::string& command)
{
    std::string all = command + "\r\n";
    send(_ControlFd,all.c_str(),all.size(),0);

    char buf[8192];
    memset(buf,0,sizeof(buf));

    //

    int n = recv(_ControlFd,buf,sizeof(buf),0);

    //

    if(n < 0)
    {
        std::cerr << "recv boom()!!!:" << std::strerror(errno) << std::endl;
        close(_ControlFd);
        _ControlFd = -1;
    }
    else if(n==0)
    {
        std::cerr << "connection closed" << std::endl;
        close(_ControlFd);
        _ControlFd = -1;
    }
    else
    {
        buf[n] = '\0';
    }

    //

    return std::string(buf);
}

std::string FTPClient::RecvResponse()   //为了不让控制通道堵塞，把226 等就是说表示输出成功的指令给接受回来吃掉
{
    char buf[8192];
    memset(buf, 0, sizeof(buf));

    //

    int n = recv(_ControlFd, buf, sizeof(buf) - 1, 0);
    if(n <= 0)
    {
        if(n < 0)
        {
            std::cerr << "RecvResponse's recv boom()!!!" << std::strerror(errno) << std::endl;
        }
        else
        {
            std::cerr << "server closed connection" << std::endl;
        }
        close(_ControlFd);
        _ControlFd = -1;
        return "";
    }

    buf[n] = '\0';
    return std::string(buf);
}

 
bool FTPClient::ParsePasv(const std::string& response,std::string& ip,int& port)
{
    size_t start = response.find('(');
    size_t end = response.find(')');
    if(start == std::string::npos || end == std::string::npos) return false;
    
    //

    int h1, h2, h3, h4, p1, p2;
    std::string nums = response.substr(start + 1, end - start - 1);
    if(sscanf(nums.c_str(), "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6)
        return false;
    
    //

    ip = std::to_string(h1) + "." + std::to_string(h2) + "." + std::to_string(h3) + "." + std::to_string(h4); // 数字变字符
    port = p1 * 256 + p2;
    return true;
}

int FTPClient::DataConnect(const std::string& ip,int port)
{
    int dataFd = socket(AF_INET,SOCK_STREAM,0);
    if(dataFd < 0)
    {
        std::cerr << "DataConnect socket boom()!!!!" << std::endl;
        return -1;
    }
    
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET,ip.c_str(),&addr.sin_addr);
    if(connect(dataFd,(sockaddr*)&addr,sizeof(addr))<0)  //大小别搞反
    {
        std::cerr << "DataConnect connect boom()!!!!" << std::endl;
        close(dataFd);
        return -1;
    }
    return dataFd;
}


void FTPClient::DoList()
{

    //拿门牌号

    std::string resp = SendCmd("PASV");
    std::cout << resp << std::endl;

    //解析IP和端口

    std::string dataIP;
    int dataPort;
    if(!ParsePasv(resp, dataIP, dataPort))
    {
        std::cerr << "ParsePasv boom()!!!" << std::endl;
        return;
    }

    //去连接数据端口

    int dataFd = DataConnect(dataIP, dataPort);
    if(dataFd < 0)
    {
        std::cerr << "DoList's DataConnect boom()!!!" << std::endl;
        return;
    }

    //让服务器把目录发过来
    std::string listResp = SendCmd("LIST");
    std::cout << listResp << std::endl;

    //看content

    char buf[4096];
    int n;
    while((n = recv(dataFd, buf, sizeof(buf), 0)) > 0)
    {
        std::cout.write(buf, n);
    }

    close(dataFd);                        // 关数据通道
    std::string finalResp = RecvResponse();  // 只接收控制通道的 226，不发送

    //
}

void FTPClient::DoGet(const std::string& filename)
{
      //门牌
      std::string resp = SendCmd("PASV");
      std::cout << resp << std::endl;

      //IP和端口
      std::string dataIP;
      int dataPort;
      if(!ParsePasv(resp, dataIP, dataPort))
      {
          std::cerr << "ParsePasv failed" << std::endl;
          return;
      }

      //敲门

      int dataFd = DataConnect(dataIP, dataPort);
      if(dataFd < 0)
      {
          std::cerr << "DataConnect failed" << std::endl;
          return;
      }

      // 让服务器"发文件过来"

      std::string retrResp = SendCmd("RETR " + filename);
      std::cout << retrResp << std::endl;

      // 本地只取文件名，不要路径 — RETR 用全路径，存盘去尾 //
      std::string local = filename; //
      size_t pos = filename.rfind('/'); //
      if(pos != std::string::npos) //
          local = filename.substr(pos + 1); //

      // 写到临时文件——避免 O_TRUNC 截断服务端正读的源文件 //

      std::string tmp = local + ".ftptmp"; //
      int fd = open(tmp.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666); //
      if(fd < 0)
      {
          std::cerr << "create file failed" << std::endl;
          close(dataFd);
          return;
      }
      //循环收数据，写给本地文件
      char buf[4096];
      int n;
      while((n = recv(dataFd, buf, sizeof(buf), 0)) > 0)
      {
          write(fd, buf, n);
      }

      //收摊

      close(fd);
      rename(tmp.c_str(), local.c_str()); // 数据完整再替换 //
      close(dataFd); //
      std::cout << RecvResponse() << std::endl; //
  }

void FTPClient::Quit()
{
    SendCmd("QUIT");         
    close(_ControlFd);         
    _ControlFd = -1;
}
void FTPClient::Close()
{
    close(_ControlFd);
    _ControlFd = -1;
}





bool FTPClient::Login(const std::string& user, const std::string& pass)
{
    std::string resp1 = SendCmd("USER " + user);
    std::cout << resp1 << std::endl;
    std::string resp2 = SendCmd("PASS " + pass);
    std::cout << resp2 << std::endl;
    return resp2[0] == '2';
}

void FTPClient::DoPut(const std::string& filename)
{
    // 先把文件读进内存——抢在服务器 truncate 之前
    std::string fileData; //
    {
        int fd = open(filename.c_str(), O_RDONLY); //
        if(fd < 0)
        {
            std::cerr << "file not found: " << filename << std::endl;
            return;
        }
        char buf[4096]; //
        int n; //
        while((n = read(fd, buf, sizeof(buf))) > 0) //
            fileData.append(buf, n); //
        close(fd); //
    }

    // 门牌依旧
    std::string resp = SendCmd("PASV");
    std::cout << resp << std::endl;
    // 依旧IP地址端口
    std::string dataIP;
    int dataPort;
    if(!ParsePasv(resp, dataIP, dataPort))
    {
        std::cerr << "ParsePasv failed" << std::endl;
        return;
    }
    // 敲门依旧
    int dataFd = DataConnect(dataIP, dataPort);
    if(dataFd < 0)
    {
        std::cerr << "DataConnect failed" << std::endl;
        return;
    }
    // 服务器只存文件名，去路径尾 — 本地读全路径，服务端只认名字 //
    std::string remote = filename; //
    size_t pos = filename.rfind('/'); //
    if(pos != std::string::npos) //
        remote = filename.substr(pos + 1); //

    // 告诉服务器"准备接收文件"
    std::string storResp = SendCmd("STOR " + remote); //
    std::cout << storResp << std::endl;

    // 内存里直接发，不依赖文件 fd
    send(dataFd, fileData.data(), fileData.size(), 0); //

    // 收摊
    shutdown(dataFd, SHUT_WR);      // 跟服务器说"我发完了"
    close(dataFd);
    std::cout << RecvResponse() << std::endl; //
}












