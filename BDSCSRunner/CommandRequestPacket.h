#pragma once
#include <string>
#include "预编译头.h"

struct CommandRequestPacket {
	// 取命令文本
	std::string toString() {			// IDA ServerNetworkHandler::handle
		std::string str = std::string(*(std::string*)((VA)this + 40));
		return str;
	}
};