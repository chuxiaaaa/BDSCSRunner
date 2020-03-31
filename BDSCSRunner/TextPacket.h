#pragma once
#include <string>
#include "预编译头.h"
struct TextPacket {
	// 取输入文本
	std::string toString() {			// IDA ServerNetworkHandler::handle
		std::string str = std::string(*(std::string*)((VA)this + 80));
		return str;
	}
};


