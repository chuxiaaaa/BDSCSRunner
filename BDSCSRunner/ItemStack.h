#pragma once
#include "Ԥ����ͷ.h"
#include "ItemStackBase.h"
struct ItemStack {
	// ȡ��ƷID
	short getId() {
		return SYMCALL(short,
			MSSYM_B1QA5getIdB1AE13ItemStackBaseB2AAA7QEBAFXZ,
			this);
	}
	// ȡ��Ʒ����ֵ
	short getAuxValue() {
		return SYMCALL(short,
			MSSYM_B1QE11getAuxValueB1AE13ItemStackBaseB2AAA7QEBAFXZ,
			this);
	}

	int getItemCount() {
		return (int)*((unsigned char*)this + 34);
	}

	std::string toString() {
		std::string str;
		SYMCALL(VA, MSSYM_MD5_7e0f26f40abf6c9395801299db2bc54f, this,&str);
		return str;
	}

	ItemStackBase* getItemBase() {
		return SYMCALL(ItemStackBase*, MSSYM_B2QQE140ItemStackBaseB2AAA4IEAAB1AA2XZ, this);
	}
	// ȡ��Ʒ����
	std::string getName() {
		std::string str;
		SYMCALL(__int64,
			MSSYM_MD5_6d581a35d7ad70fd364b60c3ebe93394,
			this, &str);
		return str;
	}
	// ȡ����������
	int getStackSize() {
		return SYMCALL(int,
			MSSYM_B1QA8getCountB1AE18ContainerItemStackB2AAA7QEBAHXZ,
			this);
	}
	// �ж��Ƿ������
	bool isNull() {
		return SYMCALL(bool,
			MSSYM_B1QA6isNullB1AE13ItemStackBaseB2AAA4QEBAB1UA3NXZ,
			this);
	}
};

