#pragma once
#include <string>
#include "‘§±‡“ÎÕ∑.h"
#include "Actor.h"
struct ActorDamageSource
{
	std::string getDeathMessage(Actor* a) {
		std::string str;
		SYMCALL(std::string,MSSYM_MD5_9d5cf510871f8161e8bd4b461f64876b,this,&str,a);
		return str;
	}
};

