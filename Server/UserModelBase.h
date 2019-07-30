#pragma once

#include "UserDO.h"
#include "MySQL.h"

// Model��ĳ�����
class UserModelBase
{
public:
	// �����û�
	virtual bool add(UserDO &user) = 0;
	// �û���¼���
	virtual bool login(UserDO &user) = 0;

	virtual bool loginOut(UserDO& user) = 0;
};

// User���Model�����
class UserModel : public UserModelBase
{
public:
	// ��дadd�ӿڷ�����ʵ�������û�����
	bool add(UserDO &user)
	{
		// ��֯sql���
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

	// ��дlogin�ӿڷ�����ʵ�ּ���¼�û����û��������Ƿ���ȷ
	bool login(UserDO &user)
	{
		// ��֯sql���
		char sql[1024] = { 0 };
		sprintf(sql, "select id from User where name='%s' and password='%s'",
			user.getName().c_str(),
			user.getPwd().c_str());

		MySQL mysql;
		if (mysql.connect())
		{
			// ע��MYSQL_RESָ����Զ��Ϊ�գ������Ƿ��ѯ������
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row = mysql_fetch_row(res);
				// MYSQL_ROW��char**��ͨ��row�Ƿ�Ϊnullptr���жϲ�ѯ�Ƿ񷵻ؽ��
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
		// ��֯sql���
		string state = "offline";
		char sql[1024] = { 0 };
		sprintf(sql, "update User set state ='%s' where id = %d",
			state.c_str(), user.getID());

		MySQL mysql;
		if (mysql.connect())
		{
			// ע��MYSQL_RESָ����Զ��Ϊ�գ������Ƿ��ѯ������
			bool res = mysql.update(sql);
			if (res)
			{
				return true;
			}
		}
		return false;
	}
};