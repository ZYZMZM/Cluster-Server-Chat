#pragma once

#include "UserDO.h"
#include "MySQL.h"

// Model层的抽象类
class UserModelBase
{
public:
	// 增加用户
	virtual bool add(UserDO &user) = 0;
	// 用户登录检查
	virtual bool login(UserDO &user) = 0;

	virtual bool loginOut(UserDO& user) = 0;
};

// User表的Model层操作
class UserModel : public UserModelBase
{
public:
	// 重写add接口方法，实现增加用户操作
	bool add(UserDO &user)
	{
		// 组织sql语句
		char sql[1024] = { 0 };
		sprintf(sql, "insert into User(name,password) values('%s', '%s')",
			user.getName().c_str(),
			user.getPwd().c_str());
			//user.getState().c_str());
		
		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))
			{
				LOG_INFO << "add User success => sql:" << sql;
				return true;
			}
		}
		LOG_INFO << "add User error => sql:" << sql;
		return false;
	}

	// 重写login接口方法，实现检查登录用户的用户名密码是否正确
	bool login(UserDO &user)
	{
		// 组织sql语句
		char sql[1024] = { 0 };
		sprintf(sql, "select id from User where name='%s' and password='%s'",
			user.getName().c_str(),
			user.getPwd().c_str());

		MySQL mysql;
		if (mysql.connect())
		{
			// 注意MYSQL_RES指针永远不为空，不管是否查询到数据
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row = mysql_fetch_row(res);
				// MYSQL_ROW是char**，通过row是否为nullptr，判断查询是否返回结果
				if (row != nullptr)
				{
					LOG_INFO << "login success => sql:" << sql;
					int id = atoi(row[0]);
					user.setID(id);
					mysql_free_result(res);
					sprintf(sql, "update User set state ='%s' where id = %d",
						user.getState().c_str(), id);
					mysql.query(sql);
					return true;
				}
			}
		}
		LOG_INFO << "login error => sql:" << sql;
		return false;
	}

	bool loginOut(UserDO& user)
	{
		// 组织sql语句
		string state = "offline";
		char sql[1024] = { 0 };
		sprintf(sql, "update User set state ='%s' where id = %d",
			state.c_str(), user.getID());

		MySQL mysql;
		if (mysql.connect())
		{
			// 注意MYSQL_RES指针永远不为空，不管是否查询到数据
			bool res = mysql.update(sql);
			if (res)
			{
				return true;
			}
		}
		return false;
	}
};