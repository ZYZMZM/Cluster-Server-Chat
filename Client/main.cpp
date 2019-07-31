#include <cstdio>
#include <iostream>
#include "ChatClient.h"

int main(int argc, char* argv[])
{
	LOG_INFO << "ChatClient run";

	muduo::net::EventLoop loop;
	muduo::net::InetAddress serverAddr(atoi(argv[1]));
	ChatClient client(&loop, serverAddr);
	client.connect();

	loop.loop();
    return 0;
}