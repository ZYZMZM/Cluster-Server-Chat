#pragma once

#include "CommonServiceBase.h"
#include <string>
#include <boost/bind.hpp>
#include <muduo/base/ThreadPool.h>
#include "RedisServer.h"
#include "UserModelBase.h"
#include "FriendModelBase.h"
#include "GroupModelBase.h"
#include "OfflineMsgModeBase.h"
#include <unordered_map>
/*
	��Ⱥ��������ʵ��
*/

class ClusterService : public CommonServiceBase
{
public:
	static ClusterService& getInstance()
	{
		static ClusterService instance;
		return instance;
	}
	ClusterService(const ClusterService&) = delete;
	ClusterService& operator=(const ClusterService&) = delete;

	// �ײ���յ�redis server�Ķ�����Ϣ�����ϱ�����������ӿ�
	void handleChannelMsg(std::string msg)
	{
		LOG_INFO << "ClusterService::handleChannelMsg:" << msg;
		json js = json::parse(msg.c_str());
		if (js["msgid"] == MSG_ONE_CHAT_ACK)
		{
			auto friendCon = Map.find(js["friendId"]);
			friendCon->second->send(js.dump());
		}
		else if (js["msgid"] == MSG_GROUP_CHAT_ACK)
		{
			auto friendCon = Map.find(js["friendId"]);
			friendCon->second->send(js.dump());
		}
	}

	// login service
	virtual void login(const muduo::net::TcpConnectionPtr& con,
		json& _js, muduo::Timestamp time)
	{
		string name = _js["username"];
		string pwd = _js["userpwd"];

		string state = "online";
		UserDO user;
		user.setID(-1);
		user.setName(name);
		user.setPassword(pwd);
		user.setState(state);

		if (userModelPtr->login(user)) // ��¼�ɹ�
		{
			LOG_INFO << "login service!name->" << name << " pwd->"
				<< pwd << " success!!!";

			// ����Ϣ�м������usrid����Ϣ
			_redisServer.subscribe(user.getID());

			json js;
			js["msgid"] = MSG_LOGIN_ACK;
			js["code"] = ACK_SUCCESS;
			// ��¼�ɹ������û���id����
			js["userid"] = user.getID();
			con->send(js.dump());
			Map.insert({ user.getID(), con });
			std::cout << "login success" << endl;
		}
		else // ��¼ʧ��
		{
			LOG_INFO << "login service!name->" << name << " pwd->"
				<< pwd << " error!!!";

			json js;
			js["msgid"] = MSG_LOGIN_ACK;
			js["code"] = ACK_ERROR;
			con->send(js.dump());
		}
	}

	// login out
	virtual void loginOut(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		string state = "offline";
		int id = js["myId"];
		std::cout << "myId : " << id << std::endl;
		UserDO user;
		user.setID(id);
		user.setState(state);
		if (userModelPtr->loginOut(user))
		{
			auto it = Map.find(id);
			Map.erase(it);
			json js;
			js["msgid"] = MSG_LOGINOUT_ACK;
			js["code"] = ACK_SUCCESS;
			con->send(js.dump());
			_redisServer.unsubscribe(id);
		}
		else
		{
			json js;
			js["msgid"] = MSG_LOGINOUT_ACK;
			js["code"] = ACK_ERROR;
			con->send(js.dump());
		}
	}

	// register service
	virtual void reg(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		string name = js["username"].get<string>();
		string pwd = js["userpwd"].get<string>();

		UserDO user;
		user.setID(-1);
		user.setName(name);
		user.setPassword(pwd);

		if (userModelPtr->add(user)) // ע��������ݳɹ�
		{
			LOG_INFO << "reg service!name->" << name << " pwd->"
				<< pwd << " success!!!";

			json js;
			js["msgid"] = MSG_REG_ACK;
			js["code"] = ACK_SUCCESS;
			con->send(js.dump());
		}
		else // ע���������ʧ��
		{
			LOG_INFO << "reg service!name->" << name << " pwd->"
				<< pwd << " error!!!";

			json js;
			js["msgid"] = MSG_REG_ACK;
			js["code"] = ACK_ERROR;
			con->send(js.dump());
		}
	}

	// add friend service
	virtual void addFriend(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		int friendId = js["friendId"].get<int>();
		int myId = js["myId"].get<int>();
		if (friendModelPtr->add(myId, friendId))
		{
			json js;
			js["msgid"] = MSG_ADD_FRIEND_ACK;
			js["code"] = ACK_SUCCESS;
			con->send(js.dump());
		}
		else
		{
			json js;
			js["msgid"] = MSG_ADD_FRIEND_ACK;
			js["code"] = ACK_ERROR;
			con->send(js.dump());
		}
	}

	// add group service
	virtual void addGroup(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		int id = js["myId"];
		string groupname = js["groupname"];

		GroupDO group;
		group.setID(-1);
		group.setName(groupname);
		group.setUsrID(id);

		if (groupModelPtr->add(group))
		{
			json jsG;
			jsG["msgid"] = MSG_ADD_GROUP_ACK;
			jsG["code"] = ACK_SUCCESS;
			jsG["name"] = groupname;
			jsG["id"] = group.getID();
			con->send(jsG.dump());
		}
		else
		{
			json jsG;
			jsG["msgid"] = MSG_ADD_GROUP_ACK;
			jsG["code"] = ACK_ERROR;
			jsG["name"] = groupname;
			jsG["id"] = group.getID();
			con->send(jsG.dump());
		}
	}

	// create group service
	virtual void createGroup(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		int id = js["myId"];
		string groupname = js["groupname"];

		GroupDO group;
		group.setID(-1);
		group.setName(groupname);
		group.setUsrID(id);

		if (groupModelPtr->createGroup(group))
		{
			json jsG;
			jsG["msgid"] = MSG_CRE_GROUP_ACK;
			jsG["code"] = ACK_SUCCESS;
			jsG["name"] = groupname;
			jsG["id"] = group.getID();
			con->send(jsG.dump());
		}
		else
		{
			json jsG;
			jsG["msgid"] = MSG_CRE_GROUP_ACK;
			jsG["code"] = ACK_ERROR;
			jsG["name"] = groupname;
			jsG["id"] = group.getID();
			con->send(jsG.dump());
		}
	}

	// one to one chat service
	virtual void oneChat(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		int friendId = js["friendId"].get<int>();
		auto friendCon = Map.find(friendId);
		string context = js["context"].get<string>();
		int myId = js["myId"].get<int>();
		if (!friendModelPtr->isFriend(myId, friendId))
		{
			json js;
			js["msgid"] = MSG_ONE_CHAT_ACK;
			js["code"] = ACK_NOT_FRI;
			js["id"] = friendId;
			con->send(js.dump());
			LOG_INFO << "Not Friend!";
		}
		else
		{
			// ������Ϣֱ�Ӵ洢�����ݿ�
			if (!userModelPtr->isOnline(friendId))
			{
				json js;
				js["msgid"] = MSG_ONE_CHAT_ACK;
				js["code"] = ACK_ERROR;
				js["time"] = time.toFormattedString();
				con->send(js.dump());

				OfflineMsg offMsg;
				offMsg.setFriendID(friendId);

				json js_O;
				js_O["conText"] = context;
				js_O["myId"] = myId;
				js_O["time"] = time.toFormattedString();
				js_O["print"] = 1;
				offMsg.setMsg(js_O);

				if (offMsglPtr->Add(offMsg))
				{
					LOG_INFO << "Offline message send success!";
				}
				else
				{
					LOG_INFO << "Offline message send failed!";
				}
			}
			else if (friendCon != Map.end())
			{
				json js;
				js["msgid"] = MSG_ONE_CHAT_ACK;
				js["code"] = ACK_SUCCESS;
				js["conText"] = context;
				js["friendId"] = friendId;
				js["myId"] = myId;
				js["time"] = time.toFormattedString();
				js["print"] = 1;
				friendCon->second->send(js.dump());
				LOG_INFO << "Send Success!";
			}
			else
			{
				json js;
				js["msgid"] = MSG_ONE_CHAT_ACK;
				js["code"] = ACK_SUCCESS;
				js["conText"] = context;
				js["friendId"] = friendId;
				js["myId"] = myId;
				js["time"] = time.toFormattedString();
				js["print"] = 1;
				_redisServer.publish(friendId, js.dump());
			}
		}
	}

	// group chat service
	virtual void groupChat(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		int groupid = js["groupId"].get<int>();
		int myId = js["myId"].get<int>();
		string context = js["context"].get<string>();

		GroupDO group;
		group.setID(groupid);
		vector<int> user = groupModelPtr->groupUser(group);

		for (int friendId : user)
		{
			auto friendCon = Map.find(friendId);
			string context = js["context"].get<string>();
			int myId = js["myId"].get<int>();

			if (!userModelPtr->isOnline(friendId))
			{
				json js;
				js["msgid"] = MSG_GROUP_CHAT_ACK;
				js["code"] = ACK_ERROR;
				js["time"] = time.toFormattedString();
				con->send(js.dump());

				OfflineMsg offMsg;
				offMsg.setFriendID(friendId);

				json js_O;
				js_O["conText"] = context;
				js_O["myId"] = myId;
				js_O["time"] = time.toFormattedString();
				js_O["print"] = 0;
				offMsg.setMsg(js_O);

				if (offMsglPtr->Add(offMsg))
				{
					//LOG_INFO << "Offline message send success!";
				}
				else
				{
					//LOG_INFO << "Offline message send failed!";
				}
			}

			else if (friendCon != Map.end())
			{
				json js;
				js["msgid"] = MSG_GROUP_CHAT_ACK;
				js["code"] = ACK_SUCCESS;
				js["conText"] = context;
				js["friendId"] = friendId;
				js["myId"] = myId;
				js["time"] = time.toFormattedString();
				js["name"] = group.getName();
				js["print"] = 0;
				friendCon->second->send(js.dump());
			}
			else
			{
				json js;
				js["msgid"] = MSG_GROUP_CHAT_ACK;
				js["code"] = ACK_SUCCESS;
				js["conText"] = context;
				js["friendId"] = friendId;
				js["myId"] = myId;
				js["time"] = time.toFormattedString();
				js["name"] = group.getName();
				js["print"] = 0;
				_redisServer.publish(friendId, js.dump());
			}
		}
	}

	// offline message service
	virtual void offlineMsg(const muduo::net::TcpConnectionPtr& con,
		json& js, muduo::Timestamp time)
	{
		int myId = js["myId"].get<int>();
		OfflineMsg offMsg;
		offMsg.setFriendID(myId);
		if (offMsglPtr->sendOffMsg(offMsg, con))
		{
			LOG_INFO << "Offline message send success!";
		}
		else
		{
			LOG_INFO << "Offline message send failed!";
		}
	}


private:
	// ���RedisServer����
	RedisServer _redisServer;
	muduo::ThreadPool _pool;

	unique_ptr<UserModelBase> userModelPtr;
	unique_ptr<FriendModelBase> friendModelPtr;
	unique_ptr<GroupModelBase> groupModelPtr;
	unique_ptr<OfflineMsgModelBase> offMsglPtr;

	// �洢��¼�û���connection��Ϣ�����ڷ���������ת����Ϣ
	std::unordered_map<int, muduo::net::TcpConnectionPtr> Map;

	ClusterService()
		: userModelPtr(new UserModel())
		, friendModelPtr(new FriendModel())
		, groupModelPtr(new GroupModel())
		, offMsglPtr(new OffMsgModel())
	{
		// ��RedisServer��ע��ͨ����Ϣ�ص�
		if (_redisServer.connect())
		{
			// ��redis���õײ��ϱ�������Ϣ�Ļص��ӿ�
			_redisServer.setChannelMsgHandler(boost::bind(&ClusterService::handleChannelMsg, this, _1));
			// �ڵ������߳�������ͨ����Ϣ����Ϊ�ò�����������
			_pool.start(1);
			_pool.run(boost::bind(&RedisServer::notifyMsg, &_redisServer));
		}
	}
};

// ȫ�ֽӿڣ�����ClusterService�����Ψһʵ��
static ClusterService& App()
{
	return ClusterService::getInstance();
}