#pragma once

#include <hiredis/hiredis.h>
#include <string>
#include <muduo/base/Logging.h>
#include <functional>

// ����redis server�������ַ��Ϣ
static std::string redisHost = "127.0.0.1";
static unsigned short port = 6379;

class RedisServer
{
public:
	// ��ʼ������redis server��context�����Ļ���
	RedisServer()
		: _context(nullptr)
		, _publishContext(nullptr)
	{}
	// �ͷź�redis server�����õ�context�����Ļ���
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
	// ����redis server
	bool connect()
	{
		// subscribe������
		this->_context = redisConnect(redisHost.c_str(), port);
		if (this->_context == nullptr || this->_context->err)
		{
			LOG_ERROR << "subscribe!redis server connect error! reason:"
				<< this->_context->errstr;
			return false;
		}
		// publish������
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
	// ������Ϣ
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

	// ����ͨ��
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

	// ȡ�����ĵ�ͨ��
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

	// ���ͨ����Ϣ��������ڵ������߳��д���ͨ��û����Ϣ��redisGetReply�������߳�
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
			// �ͷ�redis server����Ӧ��Դ
			freeReplyObject(reply);
		}
	}

	// ����ͨ����Ϣ�ϱ��Ļص���������RedisServer����ж��ĵ���Ϣ��������ͨ��Ԥ�����õ�handler�ϱ������
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