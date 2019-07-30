#pragma once

#include "MySQL.h"

// Model��ĳ�����
class FriendModelBase
{
public:
	// ���Ӻ�����Ϣ
	virtual bool add(int userid, int friendid) = 0;

	// �����ж�
	virtual bool isFriend(int userid, int friendid) = 0;

	// �Ϸ����ж�
	virtual bool isVaild(int friendid) = 0;
};

// Friend���Model�����
class FriendModel : public FriendModelBase
{
public:
	bool add(int userid, int friendid)
	{
		if (!isVaild(friendid))
		{
			return false;
		}

		// ��֯sql���
		char sql[1024] = { 0 };
		sprintf(sql, "insert into Friend(userid,friendid) values(%d, %d)",
			userid, friendid);

		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))
			{
				LOG_INFO << "add Friend success => sql:" << sql;
			}
			else
			{
				LOG_INFO << "add Friend Failed => sql:" << sql;
				return false;
			}
		}

		sprintf(sql, "insert into Friend(userid,friendid) values(%d, %d)",
			friendid, userid);
		mysql.update(sql);
		LOG_INFO << "add Friend success => sql:" << sql;
		return true;
	}

	bool isFriend(int userid, int friendid)
	{
		char sql[1024] = { 0 };
		sprintf(sql, "select * from Friend where userid = %d and friendid = %d",
			userid, friendid);

		MySQL mysql;
		mysql.connect();

		mysql_real_query(mysql._conn, sql, strlen(sql));
		MYSQL_RES* res = mysql_store_result(mysql._conn);
		const long long unsigned int rowcount = mysql_num_rows(res);
		if (rowcount != 0)
		{
			return true;
		}

		return false;
	}

	bool isVaild(int friendid)
	{
		char sql[1024] = { 0 };
		sprintf(sql, "select * from User where id = %d",
			friendid);

		MySQL mysql;
		mysql.connect();

		mysql_real_query(mysql._conn, sql, strlen(sql));
		MYSQL_RES* res = mysql_store_result(mysql._conn);
		const long long unsigned int rowcount = mysql_num_rows(res);
		if (rowcount != 0)
		{
			return true;
		}

		return false;
	}
};