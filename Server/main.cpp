#include <iostream>
using namespace std;

#include "UserDO.h"
#include "UserModelBase.h"
#include "ChatServer.h"

int main()
{
	LOG_INFO << "main pid:" << getpid();
	LOG_INFO << "main tid:" << pthread_self();
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(9995);
	ChatServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	
    return 0;
}