#pragma once

#include <string>
using namespace std;

// AllGroup表的领域模型
class GroupDO
{
public:
	GroupDO() {}
	void setID(int id) { _id = id; }
	void setName(string name) { _groupname = name; }
	void setDesc(string desc) { _groupdesc = desc; }
	void setUsrID(int id) { _usrId = id; }

	int getID()const { return _id; }
	string getName()const { return _groupname; }
	string getDesc()const { return _groupdesc; }
	int getUserID()const { return _usrId; }

private:
	int _id;
	int _usrId;
	string _groupname;
	string _groupdesc;
};