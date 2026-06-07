//The main construction of 这个代码 before 2026.05.28 , 剩下的部分更新与修复会标注出来，以便我更好复盘该项目
#include"Session.h"

#include<unistd.h>
#include<cstring>
#include<sys/socket.h>
#include<string>
#include<ctype.h>
#include<iostream>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>

//5.31 update - get 中未考虑 size 等参数，导致get不停502-no found！！！ boom() !
#include<sys/stat.h>                                                 



//构造函数，重置ClientFd , RootDir , DataFd
Session::Session(int ClientFd,const std::string& RootDir)
    :_ClientFd(ClientFd)
    ,_RootDir(RootDir)
    ,_DataFd(-1)
{
}


//析构函数，结束当前命令分析板块，关闭文件描述符 
Session::~Session()
{
    if(_DataFd != -1)
    {
        close(_DataFd);
    }
    close(_ClientFd);               ///?!
}


//运行 ]->~<-[ core
void Session::run()
{

    //进run就是成功
    
    const char* MessageBegin = "220 FTP server already ready >~<\r\n";
    send(_ClientFd,MessageBegin,strlen(MessageBegin),0);

    //对缓冲区处理一下（就是一对\r\n命令 掐头（划掉）  去尾->加\0 ， 中间也剖开分类）  这里recv返回的是实际接受到的字节数

    char buf[1024];
    int n = 0;
    while((n=recv(_ClientFd,buf,sizeof(buf) - 1,0)) > 0)
    {
        buf[n]='\0';
        
        //

        if(n>=2&&buf[n-1]=='\n'&&buf[n-2]=='\r')
        {
            buf[n-2]='\0';
        }

        else if(n>=1&&buf[n-1]=='\n')
        {

            buf[n-1]='\0';
        }//有的只发\n   (用xxd看没有0d  只有0a：\n)
        printf("客户端:%s\n",buf);
        //const char* response = "502 command not implemented \r\n";
        //send(_ClientFd,response,strlen(response),0);

        //

        char* FindSpace = strchr(buf,' ');  
        char* cmd = buf;
        char* num = nullptr; 
        if(FindSpace != nullptr)
        {
            *FindSpace = '\0';
            num = FindSpace + 1;
        }
        for(;*cmd != '\0';cmd++)
        {
            *cmd = toupper(*cmd);
        }
        if(strcmp(buf,"USER")==0)
        {
            _Username = num?num:"";
            const char* response = "331 Password required:\r\n";
            send(_ClientFd,response,strlen(response),0);
            continue;
        }
        else if(strcmp(buf,"PASS")==0)
        {
            if(num && strcmp(num,"20260501")==0)
            {
                _WhetherLogin = true;
                const char* PasswordGreat = "230 Your Password is perfect!now log in successfully!\r\n";
                send(_ClientFd,PasswordGreat,strlen(PasswordGreat),0);
                continue;
            }
            else
            {
                const char* Wrong = "530 Sorry,Please try again!\r\n";
                send(_ClientFd,Wrong,strlen(Wrong),0);
                continue;
            }   
        }
        else if(strcmp(buf,"QUIT") == 0)
        {
            const char* bye = "221 ByeBye >~<\r\n";
            send(_ClientFd,bye,strlen(bye),0);
            close(_ClientFd);
            break;
        }
    if(!_WhetherLogin)
    {
        const char* Wrong = "530 Sorry,Please try again!\r\n";
        send(_ClientFd,Wrong,strlen(Wrong),0);
    }
    else
    {
        if(strcmp(buf,"PWD")==0)
        {
            char cwd[1024];
            getcwd(cwd,sizeof(cwd));
            char MessagePWD[1200];
            snprintf(MessagePWD,sizeof(MessagePWD),"257 \"%s\" \r\n",cwd);
            send(_ClientFd,MessagePWD,strlen(MessagePWD),0);
        }

        //else if(strcmp(buf,"LET'S 狗!") == 0)
        //{
        //    const char* LanHuaCao = "200 这么能串让你串完了呗!>`'`<\r\n"; //200：命令成功
        //    send(_ClientFd,LanHuaCao,strlen(LanHuaCao),0);
        //}  //ftp不认识，无法实现

        else if(strcmp(buf,"CWD")==0)
        {
            if(chdir(num)==0)
            {
                const char* message2 = "250 changed now directory successfully!\r\n";
                send(_ClientFd,message2,strlen(message2),0);
            }
            else
            {
                const char* message2 = "550 Failed to change the directory!\r\n";
                send(_ClientFd,message2,strlen(message2),0);
            }            
        }
        else if(strcmp(buf,"TYPE")==0)
        {
            if(num && strcmp(num,"I")==0)
            {
                const char* message3 = "200 Type set to I\r\n";
                send(_ClientFd,message3,strlen(message3),0);
            }
            else if(num && strcmp(num,"A")==0)
            {
                const char* message3 = "200 Type set to A\r\n";
                send(_ClientFd,message3,strlen(message3),0);
            }
            else
            {
                const char* message3 = "504 Type not supported\r\n";
                send(_ClientFd,message3,strlen(message3),0);
            }
        }
        



        
        // 2026.05.31 update 告诉ftp服务器我的系统类型是Unix 
        else if(strcmp(buf,"SYST")==0)                                  
        {                                                               
            const char* msg = "215 UNIX Type: L8\r\n";                 
            send(_ClientFd,msg,strlen(msg),0);                          
        }
        // 2026.5.31 update  告诉ftp服务器我支持看文件的大小(拓展功能)                                                               
        else if(strcmp(buf,"FEAT")==0)                                   
        {                                                                
            const char* msg = "211-Features:\r\n SIZE\r\n211 End\r\n";  
            send(_ClientFd,msg,strlen(msg),0);                           
        }
        
        //


        else if(strcmp(buf,"PASV")==0)
        {
            if(_DataFd != -1)
            {
                close(_DataFd);
            }
            _DataFd = socket(AF_INET,SOCK_STREAM,0);
            if(_DataFd < 0)
            {
                std::cerr << "_DataFd socket boom!!!" << std::strerror(errno)<< std::endl;
            }
            

            //

            struct sockaddr_in DataAddr;
            memset(&DataAddr,0,sizeof(DataAddr));
            DataAddr.sin_family = AF_INET;
            DataAddr.sin_port = htons(0);
            DataAddr.sin_addr.s_addr = htonl(INADDR_ANY);

            //

            int bindRet = bind(_DataFd,(sockaddr*)&DataAddr,sizeof(DataAddr));
            if(bindRet < 0)
            {
                std::cerr << "DataFd bind boom!!!!!!!!" << std::strerror(errno) << std::endl;
                close(_DataFd);                                          // CC
                _DataFd = -1;                                            // CC
                const char* errMsg = "425 Can't open data connection\r\n";
                send(_ClientFd,errMsg,strlen(errMsg),0);
            }
            else
            {
                int listenRet = listen(_DataFd,1);//  0不排队，1排队
                if(listenRet < 0)
                {
                    std::cerr << "DataFd listen boom!!!!!!!!" << std::strerror(errno) << std::endl;
                }

                //

                socklen_t AddrLen = sizeof(DataAddr);
                getsockname(_DataFd,(sockaddr*)&DataAddr,(socklen_t*)&AddrLen);
                uint16_t DataPort = ntohs(DataAddr.sin_port);

                //                                                                   //////////到这里

                struct sockaddr_in LocalAddr;
                socklen_t LocalLen = sizeof(LocalAddr);
                getsockname(_ClientFd,(sockaddr*)&LocalAddr,(socklen_t*)&LocalLen);
                const char* IPStr = inet_ntoa(LocalAddr.sin_addr);
                uint8_t p1 = DataPort / 256; //高位字节
                uint8_t p2 = DataPort % 256; //低位
                //p1*256 + p2

                //

                char IPstandard[16];
                strcpy(IPstandard,IPStr);
                for(int i = 0;IPstandard[i];i++)
                {
                    if(IPstandard[i] == '.')
                    {
                        IPstandard[i] = ',';
                    }
                }

                //

                char PasvMsg[128];
                snprintf(PasvMsg,sizeof(PasvMsg),"227 Entering Passive Mode (%s,%u,%u)\r\n",IPstandard,p1,p2);
                send(_ClientFd,PasvMsg,strlen(PasvMsg),0);
            }
        }

        //

        else if(strcmp(buf,"LIST") == 0)
        {
            //检查一下是否有Fd来连接我的_DataFd   - 2026.06.04 update             
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(_DataFd,&readfds);
            struct timeval timeout;
            timeout.tv_sec = 30;
            timeout.tv_usec = 0;
            int ret = select(_DataFd+1,&readfds,nullptr,nullptr,&timeout);
            if(ret <= 0)
            {
                const char* wrong = "425 Your _DataFd none cares ! byebye! \r\n";
                send(_ClientFd,wrong,strlen(wrong),0);
                close(_DataFd);
                _DataFd = -1;
                continue;
            }

            //

            int DataClientFd = accept(_DataFd,nullptr,nullptr);
            if(DataClientFd < 0)
            {
                const char* msg = "425 Can't open data connection\r\n";
                send(_ClientFd,msg,strlen(msg),0);
            }
            else
            {
                const char* OpenMsg = "150 Opening ASCII mode data connection\r\n";
                send(_ClientFd,OpenMsg,strlen(OpenMsg),0);

                //

                FILE* fd = popen("ls -l","r");
                char line[1024];
                while(fgets(line,sizeof(line),fd))
                {
                    send(DataClientFd,line,strlen(line),0);
                }

                //

                pclose(fd);
                close(DataClientFd);

                //

                const char* DoneMsg = "226 Transfer complete\r\n";
                send(_ClientFd,DoneMsg,strlen(DoneMsg),0);
                close(_DataFd);
                _DataFd = -1;
            }
        }
        else if(strcmp(buf,"RETR") == 0 && num)//从服务器传文件给客户端 get 文件相对路径 文件存放位置相对路径
        {
            //检查一下是否有Fd来连接我的_DataFd   - 2026.06.04 update             
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(_DataFd,&readfds);
            struct timeval timeout;
            timeout.tv_sec = 30;
            timeout.tv_usec = 0;
            int ret = select(_DataFd+1,&readfds,nullptr,nullptr,&timeout);
            if(ret <= 0)
            {
                const char* wrong = "425 Your _DataFd none cares ! byebye! \r\n";
                send(_ClientFd,wrong,strlen(wrong),0);
                close(_DataFd);
                _DataFd = -1;
                continue;
            }

            //


            printf("检查: about to accept on _DataFd:%d\n for cmd:%s\n file=%s\n",_DataFd,buf,num ? num : "none"); //6.3 update 因为accept导致系统阻塞，排查情况
            int DataClientFd = accept(_DataFd,nullptr,nullptr);
            printf("检查: Let's me see the _DataClientFd:%d\n",DataClientFd); // 2026.06.03 update 

            if(DataClientFd < 0) 
            {
                const char* msg = "425 Can't open data connection\r\n";
                send(_ClientFd,msg,strlen(msg),0);
            }
            else
            {
                const char* OpenMsg = "150 Opening binary mode data connection\r\n";
                send(_ClientFd,OpenMsg,strlen(OpenMsg),0);

                //

                char NowPath[1024];
                getcwd(NowPath,sizeof(NowPath));
                printf("get - What we now working path (Server path) was : %s\n what I find was %s\n",NowPath,num);
                
                //get与put目录路径我经常搞混，当我输入错误后，加命令来复盘一下（这个命令会显示在服务器终端）

                FILE* fd = fopen(num,"rb"); //以二进制只读模式打开文件，防止损坏
                if(fd)
                {
                    char buf[4096];
                    size_t n = 0;
                    while((n = fread(buf,1,sizeof(buf),fd)) > 0)
                    {
                        send(DataClientFd,buf,n,0);
                    }
                    fclose(fd);
                    close(DataClientFd);

                    //

                    const char* DoneMsg = "226 Transfer complete\r\n";
                    send(_ClientFd, DoneMsg, strlen(DoneMsg), 0);
                }
                else
                {
                    close(DataClientFd);
                    const char* ErrMsg = "550 File not found\r\n";
                    send(_ClientFd, ErrMsg, strlen(ErrMsg), 0);
                }
            }
            close(_DataFd);
            _DataFd = -1;
        }
        else if(strcmp(buf,"STOR") == 0 && num) //从客户端接收文件并保存 put
        {
            //检查一下是否有Fd来连接我的_DataFd   - 2026.06.04 update             
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(_DataFd,&readfds);
            struct timeval timeout;
            timeout.tv_sec = 30;
            timeout.tv_usec = 0;
            int ret = select(_DataFd+1,&readfds,nullptr,nullptr,&timeout);
            if(ret <= 0)
            {
                const char* wrong = "425 Your _DataFd none cares ! byebye! \r\n";
                send(_ClientFd,wrong,strlen(wrong),0);
                close(_DataFd);
                _DataFd = -1;
                continue;
            }

            //

            int DataClientFd = accept(_DataFd,nullptr,nullptr);
            if(DataClientFd < 0)
            {
                const char* msg = "425 Can't open data connection\r\n";
                send(_ClientFd,msg,strlen(msg),0);
            }
            else
            {
                const char* Message1000 = "150 Opening binary mode data connection\r\n";
                send(_ClientFd,Message1000,strlen(Message1000),0);

                //
                char NowPath[1024];
                getcwd(NowPath,sizeof(NowPath));
                printf("put - What we now working path (Client's path) was : %s\n what I find was %s\n",NowPath,num);
                
                //STOR - put 是客户端向服务器上传文件，所以先看客户端的路径为根本
                
                FILE* fd = fopen(num,"wb");
                if(fd)
                {
                    int CommandsJudge = 0;
                    char buf[4096];
                    ssize_t n = 0;
                    while((n = recv(DataClientFd,buf,sizeof(buf),0)) > 0)
                    {
                        fwrite(buf,1,n,fd);
                    }
                    fclose(fd);
                    close(DataClientFd);

                    //

                    char HashCmd[1100];
                    for(int j = 0;num[j];j++)
                    {
                        if(!isalnum(num[j]) && num[j] != '/' && num[j] != '.' && num[j] != '_' && num[j] != '-')
                        {
                            const char* Boom = "550  your commands are not allowed! \r\n";//防止服务器被攻击
                            send(_ClientFd,Boom,strlen(Boom),0);
                            CommandsJudge = 1;
                        }
                    }
                    if(!CommandsJudge)
                    {
                        snprintf(HashCmd,sizeof(HashCmd),"md5sum %s",num);  
                        FILE* fp = popen(HashCmd,"r");
                        char HashResult[256] = "";
                        if(fp)
                        {
                            fgets(HashResult,sizeof(HashResult),fp);
                            pclose(fp);
                        }

                        //

                        char DoneMsg[1400];
                        snprintf(DoneMsg,sizeof(DoneMsg),"226 Transfer complete. MD5: %s\r\n",HashResult);
                        send(_ClientFd,DoneMsg,strlen(DoneMsg),0);
                    }
                }
                else
                {
                    close(DataClientFd);
                    const char* ErrMsg = "550 Failed to create file\r\n";
                    send(_ClientFd,ErrMsg,strlen(ErrMsg),0);
                }
            }
            close(_DataFd);
            _DataFd = -1;
        }
        


        
        //
        
        //2026.5.31 update 那个get命令疯狂502 ，却了个SIZE一直报错，那就加一下
        else if(strcmp(buf,"SIZE") == 0 && num)                          
        {                                                                

            struct stat st;                       //存文件信息的                       
            if(stat(num,&st) == 0 && S_ISREG(st.st_mode))       //         
            {                                                            
                char SizeMsg[64];                                       
                snprintf(SizeMsg,sizeof(SizeMsg),"213 %ld\r\n",(long)st.st_size); 
                send(_ClientFd,SizeMsg,strlen(SizeMsg),0);               
            }                                                           
            else                                                         
            {                                                            
                const char* ErrMsg = "550 File not found\r\n";           
                send(_ClientFd,ErrMsg,strlen(ErrMsg),0);                
            }                                                            
        }                                                                



        //

        else
        {
            const char* message = "502 Not-found!!!\r\n";
            send(_ClientFd,message,strlen(message),0);
        }
    }

    }









}

