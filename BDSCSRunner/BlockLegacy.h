#pragma once
#include <string>
#include "预编译头.h"
struct BlockLegacy {
	// 获取方块名
	auto getFullName() const {				// IDA BlockLegacy::~BlockLegacy
		return (std::string&) * (__int64*)((__int64)this + 104);
	}
	// 获取方块ID号
	auto getBlockItemID() const {
		return SYMCALL(short,
			MSSYM_B1QE14getBlockItemIdB1AE11BlockLegacyB2AAA7QEBAFXZ,
			this);
	}
};
