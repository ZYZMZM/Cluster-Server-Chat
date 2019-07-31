#pragma once
#pragma once

#include "OfflineMsg.h"
#include "MySQL.h"

// Model层的抽象类
class OfflineMsgModelBase
{
public:
	// 增加离线消息
	virtual bool Add(OfflineMsg& Omsg) = 0;

	// 向用户发送离线消息
	virtual bool sendOffMsg(OfflineMsg& Omsg, const muduo::net::TcpConnectionPtr& con) = 0;
};

// OfflineMessage表的Model层操作
class OffMsgModel : public OfflineMsgModelBase
{
public:

	bool Add(OfflineMsg& Omsg)
	{
		int id = Omsg.setFriendID();
	
		char sql[1024] = { 0 };
		json js = Omsg.getMsg();
		int myId = js["myId"];
		string context = js["conText"];
		string date = js["time"];
		sprintf(sql,
			"INSERT  INTO `OfflineMessage`(`userid`,`message`) VALUES (%d,'{\"id\": %d, \"context\": \"%s\", \"date\": \"%s\"}')",
			id, myId, context.c_str(), date.c_str());

		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))
			{
				LOG_INFO << "add OfflineMessage success => sql:" << sql;
				return true;
			}
		}
		LOG_INFO << "add OfflineMessage error => sql:" << sql;
		return false;
	}

	bool sendOffMsg(OfflineMsg& Omsg, const muduo::net::TcpConnectionPtr& con)
	{
		MySQL mysql;
		mysql.connect();
		MYSQL_ROW row;

		int id = Omsg.setFriendID();
		char sql1[1024] = { 0 };
		

		string field1("date");
		sprintf(sql1,
			"select json_extract(message,'$.%s') from OfflineMessage where userid = %d",
			field1.c_str(), id);

		mysql_real_query(mysql._conn, sql1, strlen(sql1));	
		MYSQL_RES* res = mysql_store_result(mysql._conn);
		int fields = mysql_num_fields(res);
		const long long unsigned int rowcount = mysql_num_rows(res);

		vector<string> vec(rowcount * 2);

		int j = 0;
		while ((row = mysql_fetch_row(res))) {
			for (int i = 0; i < fields; i++) {
				string str = row[i];
				str = str.substr(1, str.length() - 2);
				string pre = vec[j];
				pre += str;
				pre.insert(pre.begin(), '[');
				vec[j] = pre;
				vec[j] += ']';
			}
			j += 2;
		}

		string field("id");
		sprintf(sql1,
			"select json_extract(message,'$.%s') from OfflineMessage where userid = %d",
			field.c_str(), id);
		res = mysql.query(sql1);
		j = 1;
		while ((row = mysql_fetch_row(res))) {
			for (int i = 0; i < fields; i++) {
				string str = row[i];
				str += ':';
				vec[j] = str;
			}
			j += 2;
		}

		string field2("context");
		sprintf(sql1,
			"select json_extract(message,'$.%s') from OfflineMessage where userid = %d",
			field2.c_str(), id);
		res = mysql.query(sql1);
		j = 1;
		while ((row = mysql_fetch_row(res))) {
			for (int i = 0; i < fields; i++) {
				string str = row[i];
				str = str.substr(1, str.length() - 2);
				string pre = vec[j];
				pre += str;
				vec[j] = pre;
			}
			j += 2;
		}

		

		json js;
		js["msgid"] = MSG_OFFMSG_ACK;
		js["msg"] = vec;
		
		sprintf(sql1,
			"delete from OfflineMessage where userid = %d",
			id);
		
		mysql.update(sql1);

		con->send(js.dump());
		return true;
	}
};