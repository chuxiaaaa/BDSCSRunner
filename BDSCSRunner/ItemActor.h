#pragma once
#include "ItemStackBase.h"
#include "Ԥ����ͷ.h"
struct ItemActor 
{
	ItemStackBase* getItemBase() {
		return SYMCALL(ItemStackBase*, MSSYM_B2QQE140ItemStackBaseB2AAA4IEAAB1AA2XZ, this + 2064);
	}
};

