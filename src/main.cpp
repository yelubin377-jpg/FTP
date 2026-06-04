#include"Server.h"
#include"Session.h"
#include<iostream>
#include<unistd.h>



int main()
{
    Server server(2100,".");
    server.run();
}
