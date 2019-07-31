#pragma once

#include <hiredis/hiredis.h>
#include <string>
#include <muduo/base/Logging.h>
#include <functional>

// 配置redis server的网络地址信息
static std::string redisHost = "127.0.0.1";
static unsigned short port = 6379;

class RedisServer
{
public:
	// 初始化连接redis server的context上下文环境
	RedisServer()
		: _context(nullptr)
		, _publishContext(nullptr)
	{}
	// 释放和redis server连接用的context上下文环境
	~RedisServer()
	{
		if (this->_context != nullptr)
		{
			redisFree(this->_context);
			this->_context = nullptr;
		}

		if (this->_publishContext != nullptr)
		{
			redisFree(this->_publishContext);
			this->_publishContext = nullptr;
		}
	}
	// 连接redis server
	bool connect()
	{
		// subscribe上下文
		this->_context = redisConnect(redisHost.c_str(), port);
		if (this->_context == nullptr || this->_context->err)
		{
			LOG_ERROR << "subscribe!redis server connect error! reason:"
				<< this->_context->errstr;
			return false;
		}
		// publish上下文
		this->_publishContext = redisConnect(redisHost.c_str(), port);
		if (this->_publishContext == nullptr || this->_publishContext->err)
		{
			LOG_ERROR << "publish!redis server connect error! reason:"
				<< this->_publishContext->errstr;
			return false;
		}

		LOG_INFO << "redis server connect success!";
		return true;
	}
	// 发布消息
	void publish(int channel, std::string message)
	{
		redisReply* reply = (redisReply*)redisCommand(this->_publishContext, "PUBLISH %d %s", channel, message.c_str());
		if (reply == nullptr)
		{
			LOG_ERROR << "publish [" << channel << "] error!";
			return;
		}
		freeReplyObject(reply);
		LOG_INFO << "publish [" << channel << "] success!";
	}

	// 订阅通道
	void subscribe(int channel)
	{
		if (redisAppendCommand(this->_context, "SUBSCRIBE %d", channel) == REDIS_OK)
		{
			int done;
			redisBufferWrite(this->_context, &done);
			LOG_INFO << "subscribe [" << channel << "] success!";
			return;
		}
		LOG_ERROR << "subscribe [" << channel << "] error!";
	}

	// 取消订阅的通道
	void unsubscribe(int channel)
	{
		if (redisAppendCommand(this->_context, "UNSUBSCRIBE %d", channel) == REDIS_OK)
		{
			int done;
			redisBufferWrite(this->_context, &done);
			LOG_INFO << "unsubscribe [" << channel << "] success!";
			return;
		}
		LOG_INFO << "unsubscribe [" << channel << "] error!";
	}

	// 检测通道消息，必须放在单独的线程中处理，通道没有消息，redisGetReply会阻塞线程
	void notifyMsg()
	{
		redisReply* reply;
		while (redisGetReply(this->_context, (void**)& reply) == REDIS_OK)
		{
			if (reply != nullptr && reply->element != nullptr)
			{
				if (reply->element[2] != nullptr
					&& reply->element[2]->str != nullptr)
				{
					LOG_INFO << reply->element[2]->str;
					_channelHandler(reply->element[2]->str);
				}
			}
			else
			{
				LOG_INFO << "redisGetReply no response!";
			}
			// 释放redis server的响应资源
			freeReplyObject(reply);
		}
	}

	// 设置通道消息上报的回调函数，当RedisServer检测有订阅的消息发生，则通过预先设置的handler上报服务层
	using channelHandler = std::function<void(std::string)>;
	void setChannelMsgHandler(channelHandler handler)
	{
		_channelHandler = handler;
	}
private:
	redisContext* _context;
	redisContext* _publishContext;
	channelHandler _channelHandler;
};