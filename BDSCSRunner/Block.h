#pragma once
#include "预编译头.h"
#include "BlockLegacy.h"
struct Block {
	// 获取源
	const BlockLegacy* getLegacyBlock() const {
		return SYMCALL(const BlockLegacy*,
			MSSYM_B1QE14getLegacyBlockB1AA5BlockB2AAE19QEBAAEBVBlockLegacyB2AAA2XZ,
			this);
	}
};

