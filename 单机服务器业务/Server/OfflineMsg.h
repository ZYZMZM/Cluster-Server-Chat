#pragma once

#include <string>
#include "json.hpp"
using json = nlohmann::json;

using namespace std;

// OfflineMessage表的领域模型
class OfflineMsg
{
public:
	OfflineMsg() {}
	void setFriendID(int id) { _id = id; }
	void setMsg(json msg) { _msg = msg; }

	int setFriendID()const { return _id; }
	json getMsg()const { return _msg; }
private:
	int _id;
	json _msg;
};