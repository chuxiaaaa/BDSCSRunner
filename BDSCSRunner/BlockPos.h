#pragma once
#include "BPos3.h"
#include "预编译头.h"
struct BlockPos {
	// 获取坐标数组头
	BPos3* getPosition() const {
		return reinterpret_cast<BPos3*>(reinterpret_cast<VA>(this));
	}
};

