#pragma once

#include "GroupDO.h"
#include "MySQL.h"
#include <iostream>

// Model层的抽象类
class GroupModelBase
{
public:
	// 增加群组信息
	virtual bool createGroup(GroupDO& group) = 0;

	virtual bool add(GroupDO& group, string groupRole = "normal") = 0;

	virtual bool isExist(GroupDO& group) = 0;

	virtual vector<int> groupUser(GroupDO& group) = 0;

	virtual void setGroupName(GroupDO& group) = 0;

	virtual void setGroupId(GroupDO& group) = 0;

};

// Group表的Model层操作
class GroupModel : public GroupModelBase
{
public:
	void setGroupId(GroupDO& group)
	{
		char sql[1024] = { 0 };
		MySQL mysql;
		mysql.connect();

		/* get group id*/
		sprintf(sql, "select id from AllGroup where groupname='%s'",
			group.getName().c_str());

		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			if (row != nullptr)
			{
				int id = atoi(row[0]);
				group.setID(id);
			}
		}
	}
	bool createGroup(GroupDO &group)
	{
		char sql[1024] = { 0 };
		MySQL mysql;
		mysql.connect();
		if (!isExist(group))
		{
			sprintf(sql, "insert into AllGroup(groupname) values('%s')",
				group.getName().c_str());
			mysql.update(sql);

			setGroupId(group);

			add(group, "creator");
			return true;
		}
		return false;
	}

	bool add(GroupDO& group, string groupRole = "normal")
	{
		char sql[1024] = { 0 };
		MySQL mysql;
		mysql.connect();
		setGroupId(group);
		if (isExist(group))
		{
			sprintf(sql, "insert into GroupUser(groupid, userid,grouprole) values(%d,%d,'%s')",
				group.getID(), group.getUserID(), groupRole.c_str());
			if (mysql.update(sql))
			{
				LOG_INFO << "add Group success => sql:" << sql;
			}
			else
			{
				LOG_INFO << "add Group error => sql:" << sql;
			}
			return true;
		}
		return false;
	}

	bool isExist(GroupDO& group)
	{
		string name = group.getName();
		char sql[1024] = { 0 };
		sprintf(sql, "select * from AllGroup where groupname = '%s'",
			group.getName().c_str());

		MySQL mysql;
		if (mysql.connect())
		{
			MYSQL_RES* res = mysql.query(sql);
			if (res)
			{
				MYSQL_ROW row = mysql_fetch_row(res);
				if (row)
				{
					return true;
				}
			}
		}
		return false;
	}

	void setGroupName(GroupDO& group)
	{
		MySQL mysql;
		mysql.connect();

		char sql[1024] = { 0 };
		sprintf(sql, "select groupname from AllGroup where id=%d",
			group.getID());

		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			if (row != nullptr)
			{
				group.setName(row[0]);
			}
		}
	}

	vector<int> groupUser(GroupDO& group)
	{
		setGroupName(group);
		MySQL mysql;
		mysql.connect();
		MYSQL_ROW row;

		char sql[1024] = { 0 };

		int groupid = group.getID();
		
		sprintf(sql, "select userid from GroupUser where groupid = %d",
			groupid);

		mysql_real_query(mysql._conn, sql, strlen(sql));
		MYSQL_RES* Res = mysql_store_result(mysql._conn);

		vector<int> vec;

		while ((row = mysql_fetch_row(Res))) {
				string str = row[0];
				vec.push_back(stoi(str));
		}

		return vec;
	}
};