#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>
#include <muduo/base/Logging.h>

/*
�������࣬����muduo�⿪��
*/
class ChatServer
{
public:
	// ��ʼ��TcpServer
	ChatServer(muduo::net::EventLoop *loop,
		const muduo::net::InetAddress &listenAddr)
		:_server(loop, listenAddr, "ChatServer")
	{
		// ͨ���������ûص�����
		_server.setConnectionCallback(bind(&ChatServer::onConnection,
			this, _1));

		_server.setMessageCallback(bind(&ChatServer::onMessage,
			this, _1, _2, _3));

		// ����EventLoop���̸߳���
		_server.setThreadNum(10);
	}

	// ����ChatServer����
	void start()
	{
		_server.start();
	}

private:
	// TcpServer�󶨵Ļص����������������ӻ������ж�ʱ����
	void onConnection(const muduo::net::TcpConnectionPtr &con);
	// TcpServer�󶨵Ļص�����������������ʱ����
	void onMessage(const muduo::net::TcpConnectionPtr &con,
		muduo::net::Buffer *buf,
		muduo::Timestamp time);
private:
	muduo::net::TcpServer _server;
};