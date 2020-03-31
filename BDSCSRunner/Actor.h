#pragma once
#include "预编译头.h"
#include "Vec3.h"
struct Actor {
	//// 取方块源
	//BlockSource* getRegion() {
	//	return *reinterpret_cast<BlockSource**>(reinterpret_cast<VA>(this) + 414 * sizeof(void*));
	//}

	// 获取生物名称信息
	std::string getNameTag() {
		return SYMCALL(std::string&,
			MSSYM_MD5_7044ab83168b0fd345329e6566fd47fd,
			this);
	}

	// 获取生物当前所处维度ID
	int getDimensionId() {
		int dimensionId;
		SYMCALL(int&,
			MSSYM_B1QE14getDimensionIdB1AA5ActorB2AAA4UEBAB1QA2AVB2QDE11AutomaticIDB1AE10VDimensionB2AAA1HB2AAA2XZ,
			this, &dimensionId);
		return dimensionId;
	}

	// 是否悬空
	const BYTE isStand() {				// IDA MovePlayerPacket::MovePlayerPacket
		return *reinterpret_cast<BYTE*>(reinterpret_cast<VA>(this) + 376);
	}

	// 获取生物当前所在坐标
	Vec3* getPos() {
		return SYMCALL(Vec3*,
			MSSYM_B1QA6getPosB1AA5ActorB2AAE12UEBAAEBVVec3B2AAA2XZ, this);
	}

	// 获取生物类型
	std::string getTypeName() {
		std::string actor_typename;
		SYMCALL(std::string&,
			MSSYM_MD5_01064f7d893d9f9ef50acf1f931d1d79,
			&actor_typename, this);
		return actor_typename;
	}

	// 获取实体类型
	int getEntityTypeId() {
		return SYMCALL(int,
			MSSYM_B1QE15getEntityTypeIdB1AA5ActorB2AAA4UEBAB1QE12AW4ActorTypeB2AAA2XZ,
			this);
		//		if (t == 1)		// 未知类型，可能是玩家
		//			return 319;
	}

	// 获取实体名称
	std::string getEntityTypeName() {
		std::string en_name;
		SYMCALL(std::string&,
			MSSYM_MD5_af48b8a1869a49a3fb9a4c12f48d5a68,
			&en_name, getEntityTypeId());
		return en_name;
	}
};