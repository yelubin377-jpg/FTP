# 				FTP服务器专属数字signals or 专属工具function

## Signals

#####  220 FTP already read  -  进run函数

##### 221 Byebye >~<  - quit

##### 257 Dir  -  ls目录  ฅ՞•ﻌ•՞ฅ

##### 331 Password required   -  用户名登录成功，开始请求密码  ٩(๑>◡<๑)۶

##### 230 Log in successfully  -  登录成功！٩(ˊᗜˋ*)و

##### 530 sorry,please try again! - 再试一次   (´｡• ᵕ •｡`)`

##### 250 changed now directory successfully - 改变工作目录成功！ヽ (✿ﾟ▽ﾟ) ノ

##### 550 failed to change the directory !  -  改变工作目录失败，再来一次  (´｡• ᵕ •｡`)

##### 200 type set to I - 成功切换到二进制模式 (๑•̀ㅂ•́)و✧

##### 200 type set to A  - 成功切换至ASCII模式(๑•̀ㅂ•́)و✧   --  成功执行命令 200

######      				二进制  vs   ASCII

###### 			     不管换行符         \r\n <-> \n 互转  

######   图片，压缩包，可执行文件          .txt / .c / .cpp / .html

##### 504 Type not supported  -  没有找到相应模式，再试试吧 (๑•̀ㅂ•́)و✧

##### 215 UNIX Type: L8 - 系统类型应答码(๑•̀ㅂ•́)و✧

##### “211-”   - 指的是我还有更多行

##### “211”    - 我说完了 (๑´ڡ`๑)

























## Functions

##### strchr - 寻找某指定字符  strchr(buf, ’ a ’);

##### toupper - 转换大小写 toupper(*cmd);

##### send - 传输语句 - send(_ClientFd,Msg,strlen(Msg),0);

##### close - 关闭；

#####  getcwd - 获取当前工作目录 - getcwd(cwd,sizeof(cwd)) - 把当前的绝对目录,路径放入到cwd中

###### recv - 把内容从内核的接受缓冲区读到制定位置(如buf) - recv(__ClientFd,buf ,sizeof(buf)-1,0_);

###### -注意，返回的是实际接受到的字节数

##### strcmp - 对比字符是否相同 - strcmp(buf,” Let’s 狗! ”);

###### snprintf - 把一串字符输入到制定数组中(可读入%d等)

###### 			 - snprintf(MessagePWD,sizeof(MessagePWD),”257 Now Path is that \ "%s\” \r\n  “);

##### chdir - 切换目录 - chdir(num);

##### cerr/strerror - 系统错误时报错 - std::cerr << “something boom!!! ” << std::strerror(errno) <<std::endl;

##### memset - 填充元素/覆盖元素进特定地址/数组 - _memset_(_&DataFd, 0 , sizeof(DataFd)_); 先全填入为0；

###### htons - host to Networks short (16位) / 本机字节序(小端)转向网络字节序(大端)

######                                                                                                - _DataAddr.sin_port = htons(0)_

##### htonl - host to Networks long (32位) -  _DataAddr.sin_addr.s_addr = htonl(INADDR_ANY)_

##### socket - 创建套接字 - socket(协议，传输方式，协议编号); - _socket(AF_INET,SOCK_STREAM,0);_

###### bind - 绑定IP与端口给特定文件描述符 - bind(文件描述符，包含IP+端口的结构体，结构体长度);

###### 	-_int bindRet = bind(DataFd,(sockaddr*)&DataAddr,sizeof(DataAddr));_

##### listen - 监听 - _int ListenRet = listen(DataFd,1);_  //0不排队，1排队

###### accept - 接受连接 - accept(文件描述符，包含IP+端口的结构体，结构体的长度);

###### 	-accept(ListenFd,(sockaddr*)&DataAddr,&ClientLen;

##### getsockname (bind对立面) - 问端口 -

```c++
				socklen_t AddrLen = sizeof(DataAddr);

​				getsockname(DataFd,(sockaddr *)&DataAddr,(socklen_t *)AddrLen);

​				uint16_t DataPort = ntohs(DataAddr.sin_port);
```



