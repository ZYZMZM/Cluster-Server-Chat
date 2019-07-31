#pragma once

#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <muduo/base/ThreadPool.h>
/*
客户端实现，基于C++ muduo网络库
*/
class ChatClient
{
public:
	ChatClient(muduo::net::EventLoop* loop,
		const muduo::net::InetAddress& addr)
		:_client(loop, addr, "ChatClient")
	{
		// 设置客户端TCP连接回调接口
		_client.setConnectionCallback(bind(&ChatClient::onConnection,
			this, _1));

		// 设置客户端接收数据回调接口
		_client.setMessageCallback(bind(&ChatClient::onMessage,
			this, _1, _2, _3));
	}
	// 连接服务器
	void connect()
	{
		_client.connect();
	}
private:
	// TcpClient绑定回调函数，当连接或者断开服务器时调用
	void onConnection(const muduo::net::TcpConnectionPtr& con);
	// TcpClient绑定回调函数，当有数据接收时调用
	void onMessage(const muduo::net::TcpConnectionPtr& con,
		muduo::net::Buffer* buf,
		muduo::Timestamp time);
	// 客户端输入界面，在单独的线程中接收用户输入进行发送操作
	void userClient(const muduo::net::TcpConnectionPtr& con);

	muduo::net::TcpClient _client;
	muduo::ThreadPool _pool;
};