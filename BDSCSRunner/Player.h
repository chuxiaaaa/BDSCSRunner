#pragma once
#include <string>
#include "符号定位.hpp"
#include "预编译头.h"
#include "Actor.h"
#include "MCUUID.h"
struct Player : Actor {
	// 取uuid
	MCUUID* getUuid() {				// IDA ServerNetworkHandler::_createNewPlayer
		return (MCUUID*)((char*)this + 3192);
	}

	// 根据地图信息获取玩家xuid
	std::string& getXuid(VA level) {
		return SYMCALL(std::string&, MSSYM_MD5_337bfad553c289ba4656ac43dcb60748,
			level, (char*)this + 3192);
	}

	// 重设服务器玩家名
	void reName(std::string name) {
		SYMCALL(void, MSSYM_B1QA7setNameB1AA6PlayerB2AAA9UEAAXAEBVB2QDA5basicB1UA6stringB1AA2DUB2QDA4charB1UA6traitsB1AA1DB1AA3stdB2AAA1VB2QDA9allocatorB1AA1DB1AA12B2AAA3stdB3AAAA1Z,
			this, name);
	}
};

