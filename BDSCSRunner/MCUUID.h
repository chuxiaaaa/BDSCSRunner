#pragma once
#include <string>
#include "Ô¤±àÒëÍ·.h"
struct MCUUID {
	// È¡uuid×Ö·û´®
	std::string toString() {
		std::string s;
		SYMCALL(std::string&, MSSYM_MD5_40e8abf6eb08f7ee446159cdd0a7f283, this, &s);
		return s;
	}
};

