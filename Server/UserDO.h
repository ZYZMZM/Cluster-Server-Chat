#pragma once

#include <string>
using namespace std;

// User表的领域模型类
class UserDO
{
public:
	UserDO() {}
	void setID(int id) { _id = id; }
	void setName(string name) { _name = name; }
	void setPassword(string pwd) { _password = pwd; }
	void setState(string state) { _state = state; }

	int getID()const { return _id; }
	string getName()const { return _name; }
	string getPwd()const { return _password; }
	string getState()const { return _state; }
private:
	int _id;
	string _name;
	string _password;
	string _state;
};
