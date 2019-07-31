#pragma once

#include <mysql.h>
#include <string>
#include <muduo/base/Logging.h>
using namespace std;

// ���ݿ�������Ϣ
static string server = "127.0.0.1";
static string user = "root";
static string password = "zyzmzm";
static string dbname = "chat";

// ���ݿ������
class MySQL
{
public:
	// ��ʼ�����ݿ�����
	MySQL()
	{
		_conn = mysql_init(nullptr);
	}
	// �ͷ����ݿ�������Դ
	~MySQL()
	{
		if (_conn != nullptr)
			mysql_close(_conn);
	}
	// �������ݿ�
	bool connect()
	{
		 MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
			password.c_str(), dbname.c_str(), 3306, nullptr, 0);
		 if (p != nullptr)
		 {
			 mysql_query(_conn, "set names gbk");
		 }
		 return p;
	}
	// ���²���
	bool update(string sql)
	{
		if (mysql_query(_conn, sql.c_str()))
		{
			LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
				<< sql << "����ʧ��!";
			return false;
		}
		return true;
	}
	// ��ѯ����
	MYSQL_RES* query(string sql)
	{
		if (mysql_query(_conn, sql.c_str()))
		{
			LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
				<< sql << "��ѯʧ��!";
			return nullptr;
		}
		return mysql_use_result(_conn);
	}

	MYSQL *_conn;
};