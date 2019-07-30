#include "ChatServer.h"
#include "SingleService.h"
#include <thread>

/*
当ChatServer接收到新连接或连接中断时，会调用该回调函数，并把连接信息封装在
TcpConnection中通过参数传递进来
*/
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& con)
{
	// muduo使用示例代码
	LOG_INFO << "ChatServer:" << con->peerAddress().toIpPort() << "->"
		<< con->localAddress().toIpPort() << " state:"
		<< (con->connected() ? "UP" : "DOWN");
	LOG_INFO << "onConnection tid:" << pthread_self();
}

/*
当ChatServer已存在连接接收到新数据时，会调用该回调函数，并把数据封装在
Buffer中通过参数传递进来
*/
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& con,
	muduo::net::Buffer* buf,
	muduo::Timestamp time)
{
	muduo::string msg(buf->retrieveAllAsString());
	json js = json::parse(msg.c_str());
	App().handler()[js["msgid"].get<int>()](con, js, time);
}