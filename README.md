# FTP 项目

PASV被动模式，端口 2100。

## 项目结构

```
FTP/
├── Client/
│   ├── FTPClient.h      # 客户端类声明
│   ├── FTPClient.cpp    # 客户端类实现
│   └── main.cpp         # 客户端入口（命令循环）
├── src/
│   ├── Server.h         # 服务端类声明
│   ├── Server.cpp       # 服务端类实现（多线程 accept）
│   ├── Session.h        # 会话类声明
│   ├── Session.cpp      # 会话处理（USER/PASS/PASV/LIST/RETR/STOR）
│   └── main.cpp         # 服务端入口
├── README.md
├── ftp_client           # 编译产物：客户端
└── server               # 编译产物：服务端
```

## 编译

```bash
#请先cd进文件夹FTP
eg:cd ~/桌面/FTP
# 服务端
g++ -std=c++11 src/main.cpp src/Server.cpp src/Session.cpp -o server

# 客户端
g++ -std=c++11 Client/main.cpp Client/FTPClient.cpp -o ftp_client
```

## 运行

**终端 1（先开服务端）：**
```bash
cd ~/桌面/FTP && ./server
```

**终端 2（再开客户端）：**
```bash
cd ~/桌面/FTP && ./ftp_client
```

## 登录

```
用户名: 任意
密码: 20260501
```

用户名任意，密码必须为 `20260501`

## 命令

| 命令 | 说明 | 示例 |
|------|------|------|
| ls | 列出服务器当前目录 | ls |
| `get 路径` | 从服务器下载文件 | `get ../1.md` |
| `put 路径` | 上传本地文件到服务器 | `put ../1.md` |
| `quit` | 退出 | `quit` |

## put / get 路径规则

**put：** 本地用完整路径读文件，服务端只存纯文件名（自动去路径尾）。

```
put ../1.md
→ 读 ../1.md（本地）
→ STOR 1.md（服务器只看到文件名）
→ 文件存入服务器工作目录
```

**get：** RETR 用完整路径请求，存盘只取纯文件名（自动去路径尾）。

```
get ../1.md
→ RETR ../1.md（服务器按路径找）
→ 存为 1.md（本地去尾，只保留文件名）
→ 文件存入客户端当前目录
```

## 完整示例

```
220 FTP server already ready >~<

用户名: A
密码: 20260501

331 Password required:
230 Your Password is perfect!

ls
→ 列出目录

put ../1.md
→ 227 → 150 → 226 Transfer complete. MD5: xxxxx
→ 桌面 1.md 已上传到服务器

get ../1.md
→ 227 → 150 → 226 Transfer complete
→ 从服务器下载 1.md 到当前目录

quit
→ 221 ByeBye
```



