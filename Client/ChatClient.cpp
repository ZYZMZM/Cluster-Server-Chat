#include "ChatClient.h"
#include "public.h"
#include <iostream>
#include <semaphore.h>
#include <string>
#include "json.hpp"
using json = nlohmann::json;
             
sem_t sem;
sem_t login_outSem;
sem_t offMsg_Sem;
sem_t addFri_Sem;
sem_t addGro_Sem;
sem_t creGro_Sem;
volatile bool loginSuccess = false;
volatile bool loginOutSuccess = false;
volatile bool addFriSuccess = false;
volatile bool addGroSuccess = false;
volatile bool creGroSuccess = false;
int MyId = 0;

// TcpClient绑定回调函数，当连接或者断开服务器时调用
void ChatClient::onConnection(const muduo::net::TcpConnectionPtr &con)
{
	/* 连接服务器成功后，开启和服务器的交互通信功能 */

	if (con->connected()) // 和服务器连接成功
	{
		LOG_INFO << "connect server success!";

		_pool.start(1);

		// 启动线程专门处理用户的输入操作
		_pool.run(bind(&ChatClient::userClient, this, con));

		sem_init(&sem, false, 0); 
		sem_init(&login_outSem, false, 0);
		sem_init(&offMsg_Sem, false, 0);
		sem_init(&addFri_Sem, false, 0);
		sem_init(&addGro_Sem, false, 0);
		sem_init(&creGro_Sem, false, 0);
		
	}
	else // 和服务器连接失败
	{

	}
}

// TcpClient绑定回调函数，当有数据接收时调用
void ChatClient::onMessage(const muduo::net::TcpConnectionPtr &con,
	muduo::net::Buffer *buf,
	muduo::Timestamp time)
{
	/* 负责接收服务器响应的消息，在客户端进行显示 */
	muduo::string msg(buf->retrieveAllAsString());
	json js = json::parse(msg.c_str());
	if (js["msgid"] == MSG_LOGIN_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			LOG_INFO << "Login Success";
			loginSuccess = true;
			MyId = js["userid"];
		}
		else
		{
			LOG_INFO << "Login Failed";
			loginSuccess = false;
		}
		sem_post(&sem);
	}
	else if (js["msgid"] == MSG_REG_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			LOG_INFO << "Reg Success";
			loginSuccess = true;
		}
		else
		{
			LOG_INFO << "Reg Failed";
			loginSuccess = false;
		}
		sem_post(&sem);
	}
	else if (js["msgid"] == MSG_ONE_CHAT_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			int friendId = js["myId"].get<int>();
			std::string context = js["conText"].get<std::string>();
			std::string time = js["time"].get<std::string>();
			std::cout << std::endl << time << std::endl;
			std::cout << friendId << " : " << context << std::endl;
		}
		else if (js["code"] == ACK_NOT_FRI)
		{
			int id = js["id"];
			std::cout << "id : " << id <<", is not your friend!" << std::endl;
		}
		else
		{
			std::string time = js["time"].get<std::string>();
			std::cout << std::endl << time << std::endl;
			std::cout << "Offline message has send!" << std::endl;
		}
	}
	else if (js["msgid"] == MSG_LOGINOUT_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			loginOutSuccess = true;
		}
		sem_post(&login_outSem);
	}
	else if (js["msgid"] == MSG_OFFMSG_ACK)
	{
		std::vector<std::string> vec = js["msg"];
		int i = 0;
		for (std::string str : vec)
		{
			if (i % 2 == 0 && i != 0)
				std::cout << std::endl;
			std::cout << str << "  ";
			++i;
		}
		std::cout << std::endl;
		sem_post(&offMsg_Sem);
	}
	else if (js["msgid"] == MSG_ADD_FRIEND_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			addFriSuccess = true;
		}
		sem_post(&addFri_Sem);
	}
	else if (js["msgid"] == MSG_ADD_GROUP_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			addGroSuccess = true;
		}
		sem_post(&addGro_Sem);
	}
	else if (js["msgid"] == MSG_GROUP_CHAT_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			int friendId = js["myId"].get<int>();
			std::string context = js["conText"].get<std::string>();
			std::string time = js["time"].get<std::string>();
			std::string name = js["name"].get<std::string>();
			std::cout << std::endl << "Group " << name << "  [" << time << "]" << std::endl;
			std::cout << friendId << " : " << context << std::endl;
		}
		else
		{
			/*std::string time = js["time"].get<std::string>();
			std::cout << std::endl << time << std::endl;
			std::cout << "Offline message has send!" << std::endl;*/
		}
	}
	else if (js["msgid"] == MSG_CRE_GROUP_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			creGroSuccess = true;
		}
		sem_post(&creGro_Sem);
	}
}

void menu()
{
	std::cout << "-------------------" << std::endl;
	std::cout << "| af - Add Friend |" << std::endl;
	std::cout << "-------------------" << std::endl;
	std::cout << "| ag - Add Group  |" << std::endl;
	std::cout << "-------------------" << std::endl;
	std::cout << "| cg - Cre Group  |" << std::endl;
	std::cout << "-------------------" << std::endl;
	std::cout << "| c - Chat        |" << std::endl;
	std::cout << "-------------------" << std::endl;
	std::cout << "| g - Group Chat  |" << std::endl;
	std::cout << "-------------------" << std::endl;
	/*std::cout << "| l - Offline Msg |" << std::endl;
	std::cout << "-------------------" << std::endl;*/
	std::cout << "| q - Quit        |" << std::endl;
	std::cout << "-------------------" << std::endl;
}

// 处理用户的输入操作
void ChatClient::userClient(const muduo::net::TcpConnectionPtr &con)
{
	int input;
	std::cout << "----------------" << std::endl;
	std::cout << "| 1 - Login     |" << std::endl;
	std::cout << "----------------" << std::endl;
	std::cout << "| 2 - Register  |" << std::endl;
	std::cout << "----------------" << std::endl;
	std::cout << "| 3 - Quit      |" << std::endl;
	std::cout << "----------------" << std::endl;
	std::cout << "please input the choice : ";
	std::cin >> input;
	for (;;)
	{
		if (input == 1)
		{
			std::cout << std::endl;
			std::cout << "*****Login*****" << std::endl;
			char username[50] = {};
			char passwd[50] = {};
			std::cout << "UserName : ";
			std::cin >> username;
			std::cout << "PassWord : ";
			std::cin >> passwd;
			std::cout << std::endl;
			json js;
			js["msgid"] = MSG_LOGIN;
			js["username"] = username;
			js["userpwd"] = passwd;
			con->send(js.dump());
			sem_wait(&sem);

			if (loginSuccess)
			{
				LOG_INFO << "The Offline Message:";
				json js;
				js["msgid"] = MSG_OFFMSG;
				js["myId"] = MyId;
				con->send(js.dump());
				sem_wait(&offMsg_Sem);
				menu();
				break;
			}
			else
			{
				LOG_INFO << "UserName or PassWord is Wrong";
				input = 1;
				continue;
			}
		}
		else if (input == 2)
		{
			std::cout << std::endl;
			std::cout << "*****Register*****" << std::endl;
			char username[50] = {};
			char passwd[50] = {};
			std::cout << "UserName : ";
			std::cin >> username;
			std::cout << "PassWord : ";
			std::cin >> passwd;
			std::cout << std::endl;
			json js;
			js["msgid"] = MSG_REG;
			js["username"] = username;
			js["userpwd"] = passwd;
			con->send(js.dump());
			sem_wait(&sem);

			if (loginSuccess)
			{
				input = 1;
				continue;
			}
			else
			{
				input = 2;
				continue;
			}
		}
		else if (input == 3)
		{
			std::cout << "Bye!" << std::endl;
			exit(0);
		}
	}

	std::string inputOP = "";
	std::cout << ">>> ";
	std::cin >> inputOP;
	// c:1:hello
	while (inputOP != "q")
	{
		if (inputOP.substr(0, 2) == "c:")
		{
			int pos = inputOP.find_first_of(':');
			std::string op = inputOP.substr(0, pos);
			int pos2 = inputOP.find_last_of(':');
			int friendId = std::stoi(inputOP.substr(pos + 1, pos2 - pos - 1));
			std::string contextStr = "";
			contextStr = inputOP.substr(pos2 + 1, inputOP.length() - pos2 - 1);

			json js;
			js["msgid"] = MSG_ONE_CHAT;
			js["myId"] = MyId;
			js["context"] = contextStr;
			js["friendId"] = friendId;
			con->send(js.dump());

			std::cin >> inputOP;
			continue;
		}
		else if (inputOP.substr(0, 2) == "g:")
		{
			int pos = inputOP.find_first_of(':');
			std::string op = inputOP.substr(0, pos);
			int pos2 = inputOP.find_last_of(':');
			int groupId = std::stoi(inputOP.substr(pos + 1, pos2 - pos - 1));
			std::string contextStr = "";
			contextStr = inputOP.substr(pos2 + 1, inputOP.length() - pos2 - 1);

			json js;
			js["msgid"] = MSG_GROUP_CHAT;
			js["myId"] = MyId;
			js["context"] = contextStr;
			js["groupId"] = groupId;
			con->send(js.dump());

			std::cin >> inputOP;
			continue;
		}
		else if (inputOP.substr(0, 2) == "af")
		{
			int pos = inputOP.find_first_of(':');
			std::string op = inputOP.substr(0, pos);
			int friendId = std::stoi(inputOP.substr(pos + 1, inputOP.length() - pos - 1));

			json js;
			js["msgid"] = MSG_ADD_FRIEND;
			js["myId"] = MyId;
			js["friendId"] = friendId;
			con->send(js.dump());
			sem_wait(&addFri_Sem);
			if (addFriSuccess)
			{
				LOG_INFO << "Add Success";
				addFriSuccess = false;
			}
			else
			{
				LOG_INFO << "Add Failed";
			}
			std::cin >> inputOP;
			continue;
		}
		else if (inputOP.substr(0, 2) == "ag")
		{
			int pos = inputOP.find_first_of(':');
			std::string op = inputOP.substr(0, pos);
			std::string groupname = inputOP.substr(pos + 1, inputOP.length() - pos - 1);

			json js;
			js["msgid"] = MSG_ADD_GROUP;
			js["myId"] = MyId;
			js["groupname"] = groupname;
			con->send(js.dump());
			sem_wait(&addGro_Sem);
			if (addGroSuccess)
			{
				LOG_INFO << "Add Group Success";
				addGroSuccess = false;
			}
			else
			{
				LOG_INFO << "Add Group Failed, Name Error Or Not Exist!";
			}
			std::cin >> inputOP;
			continue;
		}
		else if (inputOP.substr(0, 2) == "cg")
		{
			int pos = inputOP.find_first_of(':');
			std::string op = inputOP.substr(0, pos);
			std::string groupname = inputOP.substr(pos + 1, inputOP.length() - pos - 1);

			json js;
			js["msgid"] = MSG_CRE_GROUP;
			js["myId"] = MyId;
			js["groupname"] = groupname;
			con->send(js.dump());
			sem_wait(&creGro_Sem);
			if (creGroSuccess)
			{
				LOG_INFO << "Create Group Success";
				creGroSuccess = false;
			}
			else
			{
				LOG_INFO << "Create Group Failed";
			}
			std::cin >> inputOP;
			continue;
		}
		else
		{
			LOG_INFO << "Error Input";
			std::cin >> inputOP;
			continue;
		}
	}

	if (inputOP == "q")
	{
		json js;
		js["msgid"] = MSG_LOGINOUT;
		js["myId"] = MyId;
		con->send(js.dump());
		sem_wait(&login_outSem);
		if (loginOutSuccess)
		{
			exit(0);
		}
	}
}