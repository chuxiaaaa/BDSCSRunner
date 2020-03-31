#include "预编译头.h"
#include "io.h"
#include "fstream"
#include "string"
#include <iostream>
#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <map>      
#include "Player.h"
#include "json/json.h"
#include "PluginInfo.h"
#include "PluginEvent.h"
#include "插件模块.h"
#include "BlockSource.h"
#include "BlockActor.h"
#include "ContainerItemStack.h"
#include "LevelContainerManagerModel.h"
#include "Mob.h"
#include "TextPacket.h"
#include "CommandRequestPacket.h"
#include "RBStream.h"
#include "VarInts.h"
#include "MyPkt.h"
#include "ModalFormResponsePacket.h"
#include <random>
#include <stdio.h> 
#include <direct.h> 

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif

using std::string;

static std::map<int, PluginInfo> plugins;
static std::map<unsigned, bool> fids;
static std::map<std::string, int> playerUseItem;
static VA p_spscqueue;
static std::map<std::string, Player*> onlinePlayers;

static std::map<Player*, VA> playerInventory;

extern "C" _declspec(dllexport) bool runcmd(char* cmd);


extern "C" _declspec(dllexport) int sendSimpleForm(char* uuid, char* title, char* content, char* buttons);

extern "C" _declspec(dllexport) int sendModalForm(char* uuid, char* title, char* content, char* button1, char* button2);

extern "C" _declspec(dllexport) int sendCustomForm(char* uuid, char* json);

extern "C" _declspec(dllexport) bool destroyForm(unsigned fid);

static bool debugModel;

static Json::Value toJson(std::string s) {
	Json::Value jv;
	Json::CharReaderBuilder r;
	JSONCPP_STRING errs;
	std::unique_ptr<Json::CharReader> const jsonReader(r.newCharReader());
	bool res = jsonReader->parse(s.c_str(), s.c_str() + s.length(), &jv, &errs);
	if (!res || !errs.empty()) {
		std::cout << "JSON转换失败.." + errs << std::endl;
	}
	return jv;
}


static unsigned getFormId() {
	unsigned id = time(0);
	do {
		--id;
	} while (id == 0 || fids[id]);
	fids[id] = true;
	return id;
}

int sendForm(VA pPlayer, std::string str)
{
	if (pPlayer == 0)
		return 0;
	auto payload = str;
	WBStream ws;
	unsigned fid = getFormId();
	ws.apply(VarUInt(fid));
	ws.apply(VarUInt(payload.size()));
	ws.write(payload.c_str(), payload.size());
	MyPkt<100, false> guipk{ ws.data };
	SYMCALL(VA, MSSYM_B1QE17sendNetworkPacketB1AE12ServerPlayerB2AAE15UEBAXAEAVPacketB3AAAA1Z,
		pPlayer, &guipk);
	return fid;
}

bool destroyForm(unsigned fid)
{
	if (fids[fid]) {
		fids.erase(fid);
		return true;
	}
	return false;
}

std::string createSimpleFormString(std::string title, std::string content, Json::Value& bttxts) {
	Json::Value jv;
	jv["type"] = "form";
	jv["title"] = title;
	jv["content"] = content;
	jv["buttons"] = bttxts;
	return jv.toStyledString();
}

std::string createModalFormString(std::string title, std::string content, std::string button1, std::string button2) {
	Json::Value jv;
	jv["type"] = "modal";
	jv["title"] = title;
	jv["content"] = content;
	jv["button1"] = button1;
	jv["button2"] = button2;
	return jv.toStyledString();
}

int sendSimpleForm(char* uuid, char* title, char* content, char* buttons) {
	if (!onlinePlayers[uuid])
		return 0;
	Json::Value bts;
	Json::Value ja = toJson(buttons);
	for (int i = 0; i < ja.size(); i++) {
		Json::Value bt;
		bt["text"] = ja[i];
		bts.append(bt);
	}
	std::string str = createSimpleFormString(title, content, bts);
	return sendForm((VA)onlinePlayers[uuid], str);
}

int sendModalForm(char* uuid, char* title, char* content, char* button1, char* button2) {
	if (!onlinePlayers[uuid])
		return 0;
	std::string str = createModalFormString(title, content, button1, button2);
	return sendForm((VA)onlinePlayers[uuid], str);
}

int sendCustomForm(char* uuid, char* json) {
	Player* p = onlinePlayers[uuid];
	if (p == NULL)
		return 0;
	return sendForm((VA)p, json);
}

static void DebugPr(float log) {
	if (debugModel)
		std::cout << log << std::endl;
}


static void DebugPr(string log) {
	if (debugModel)
		std::cout << log << std::endl;
}

bool runcmd(char* cmd) {
	if (p_spscqueue != 0)
		return SYMCALL(bool, MSSYM_MD5_b5c9e566146b3136e6fb37f0c080d91e, p_spscqueue, new std::string(cmd));
	return false;
}

static std::string toJsonString(Json::Value v) {
	Json::StreamWriterBuilder w;
	std::ostringstream os;
	std::unique_ptr<Json::StreamWriter> jsonWriter(w.newStreamWriter());
	jsonWriter->write(v, &os);
	return std::string(os.str());
}

const char* BeforeOnServerCmd = u8"BeforeOnServerCmd";
const char* AfterOnServerCmd = u8"AfterOnServerCmd";
const char* BeforeOnServerCmdOutput = u8"BeforeOnServerCmdOutput";
const char* AfterOnServerCmdOutput = u8"AfterOnServerCmdOutput";
const char* BeforeOnFormSelect = u8"BeforeOnFormSelect";
const char* AfterOnFormSelect = u8"AfterOnFormSelect";
const char* BeforeOnUseItem = u8"BeforeOnUseItem";
const char* AfterOnUseItem = u8"AfterOnUseItem";
const char* BeforeOnMove = u8"BeforeOnMove";
const char* AfterOnMove = u8"AfterOnMove";
const char* BeforeOnAttack = u8"BeforeOnAttack";
const char* AfterOnAttack = u8"AfterOnAttack";
const char* BeforeOnPlacedBlock = u8"BeforeOnPlacedBlock";
const char* AfterOnPlacedBlock = u8"AfterOnPlacedBlock";
const char* BeforeOnDestroyBlock = u8"BeforeOnDestroyBlock";
const char* AfterOnDestroyBlock = u8"AfterOnDestroyBlock";
const char* BeforeOnStartOpenChest = u8"BeforeOnStartOpenChest";
const char* AfterOnStartOpenChest = u8"AfterOnStartOpenChest";
const char* BeforeOnStartOpenBarrel = u8"BeforeOnStartOpenBarrel";
const char* AfterOnStartOpenBarrel = u8"AfterOnStartOpenBarrel";
const char* BeforeOnChangeDimension = u8"BeforeOnChangeDimension";
const char* AfterOnChangeDimension = u8"AfterOnChangeDimension";
const char* BeforeOnLoadName = u8"BeforeOnLoadName";
const char* AfterOnLoadName = u8"AfterOnLoadName";
const char* BeforeOnPlayerLeft = u8"BeforeOnPlayerLeft";
const char* AfterOnPlayerLeft = u8"AfterOnPlayerLeft";
const char* BeforeOnStopOpenChest = u8"BeforeOnStopOpenChest";
const char* AfterOnStopOpenChest = u8"AfterOnStopOpenChest";
const char* BeforeOnStopOpenBarrel = u8"BeforeOnStopOpenBarrel";
const char* AfterOnStopOpenBarrel = u8"AfterOnStopOpenBarrel";
const char* BeforeOnSetSlot = u8"BeforeOnSetSlot";
const char* AfterOnSetSlot = u8"AfterOnSetSlot";
const char* BeforeOnNamedMobDie = u8"BeforeOnNamedMobDie";
const char* AfterOnNamedMobDie = u8"AfterOnNamedMobDie";
const char* BeforeOnRespawn = u8"BeforeOnRespawn";
const char* AfterOnRespawn = u8"AfterOnRespawn";
const char* BeforeOnChat = u8"BeforeOnChat";
const char* AfterOnChat = u8"AfterOnChat";
const char* BeforeOnInputText = u8"BeforeOnInputText";
const char* AfterOnInputText = u8"AfterOnInputText";
const char* BeforeOnInputCommand = u8"BeforeOnInputCommand";
const char* AfterOnInputCommand = u8"AfterOnInputCommand";
const char* BeforeOnExploed = u8"BeforeOnExploed";
const char* AfterOnExploed = u8"AfterOnExploed";
const char* BeforeOnPlayerJump = u8"BeforeOnPlayerJump";
const char* AfterOnPlayerJump = u8"AfterOnPlayerJump";
const char* BeforeOnRaidStart = u8"BeforeOnRaidStart";
const char* AfterOnRaidStart = u8"AfterOnRaidStart";
const char* BeforeOnRaidEnd = u8"BeforeOnRaidEnd";
const char* AfterOnRaidEnd = u8"AfterOnRaidEnd";
const char* BeforeOnEntityFall = u8"BeforeOnPlayerFall";
const char* AfterOnEntityFall = u8"AfterOnPlayerFall";

static void addPlayerInfo(Json::Value& jv, Player* p) {
	if (p) {
		jv["playername"] = p->getNameTag();
		jv["isstand"] = p->isStand();
		jv["XYZ"] = toJson(p->getPos()->toJsonString());
	}
}

static VA p_level;

bool InvokeEvent(const char* eventName, char* param, Json::Value* root) {
	Json::Reader reader;
	for (auto i = plugins.begin(); i != plugins.end(); ++i)
	{
		try
		{
			typedef char* (*InvokeEvent)(const char*);
			InvokeEvent beforeLoadName = InvokeEvent(GetProcAddress(i->second.pluginHandle, eventName));
			if (beforeLoadName != NULL)
			{
				char* ret = (*beforeLoadName)(param);
				if (reader.parse(ret, *root)) {
					if ((*root)["intercept"].asString() == "true") {
						DebugPr(i->second.pluginName + u8"拦截" + eventName);
						return true;
					}
				}
			}
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}
	}
	return false;
}

THook(VA, MSSYM_MD5_3b8fb7204bf8294ee636ba7272eec000,
	VA _this) {
	p_spscqueue = original(_this);
	return p_spscqueue;
}

THook(Player*, MSSYM_MD5_c4b0cddb50ed88e87acce18b5bd3fb8a,
	Player* _this, VA level, __int64 a3, int a4, __int64 a5, __int64 a6, void* uuid, std::string& struuid, __int64* a9, __int64 a10, __int64 a11) {
	p_level = level;
	return original(_this, level, a3, a4, a5, a6, uuid, struuid, a9, a10, a11);
}

THook(bool,
	MSSYM_MD5_b5c9e566146b3136e6fb37f0c080d91e,
	VA _this, std::string* cmd) {
	Json::Value jv;
	jv["cmd"] = *cmd;
	Json::Value root;
	if (jv["cmd"] == "debug") {
		debugModel = !debugModel;
		if (debugModel)
		{
			std::cout << "[BDSCSRunner]debugOn" << std::endl;
		}
		else
		{
			std::cout << "[BDSCSRunner]debugOff" << std::endl;
		}
		return false;
	}
	if (!InvokeEvent(BeforeOnServerCmd, toJsonString(jv).data(), &root)) {
		bool result = original(_this, cmd);
		root.clear();
		InvokeEvent(AfterOnPlacedBlock, toJsonString(jv).data(), &root);
		return result;
	}
	return false;
}

static const VA STD_COUT_HANDLE = SYM_OBJECT(VA,
	MSSYM_B2UUA3impB2UQA4coutB1AA3stdB2AAA23VB2QDA5basicB1UA7ostreamB1AA2DUB2QDA4charB1UA6traitsB1AA1DB1AA3stdB3AAAA11B1AA1A); 

THook(VA,
	MSSYM_MD5_b5f2f0a753fc527db19ac8199ae8f740,
	VA handle, char* str, VA size) {
	if (handle == STD_COUT_HANDLE) {
		Json::Value jv;
		jv["output"] = std::string(str);
		Json::Value root;
		if (!InvokeEvent(BeforeOnServerCmdOutput, toJsonString(jv).data(), &root)) {
			VA result = original(handle, str, size);
			root.clear();
			InvokeEvent(AfterOnServerCmdOutput, toJsonString(jv).data(), &root);
			return result;
		}
		return handle;
	}
	return original(handle, str, size);
}

THook(void,
	MSSYM_MD5_8b7f7560f9f8353e6e9b16449ca999d2,
	VA _this, VA id, VA handle, ModalFormResponsePacket** fp) {
	ModalFormResponsePacket* fmp = *fp;
	Player* p = SYMCALL(Player*, MSSYM_B2QUE15getServerPlayerB1AE20ServerNetworkHandlerB2AAE20AEAAPEAVServerPlayerB2AAE21AEBVNetworkIdentifierB2AAA1EB1AA1Z,
		handle, id, *(char*)((VA)fmp + 16));
	if (p != NULL) {
		UINT fid = fmp->getFormId();
		if (destroyForm(fid)) {
			Json::Value jv;
			Json::Value root;
			addPlayerInfo(jv, p);
			jv["uuid"] = p->getUuid()->toString();
			jv["formid"] = fid;
			jv["selected"] = fmp->getSelectStr();		
			if (!InvokeEvent(BeforeOnFormSelect, toJsonString(jv).data(), &root)) {
				original(_this, id, handle, fp);
				root.clear();
				InvokeEvent(AfterOnFormSelect, toJsonString(jv).data(), &root);
			}
			return;
		}
	}
	original(_this, id, handle, fp);
}

THook(__int64,
	MSSYM_MD5_949c4cd05bf2b86d54fb93fe7569c2b8,
	void* _this, Player* pPlayer, const Block* pBlk, BlockPos* pBlkpos, bool _bool) {
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	jv["blockid"] = pBlk->getLegacyBlock()->getBlockItemID();
	jv["blockname"] = pBlk->getLegacyBlock()->getFullName();
	Json::Value root;
	if (!InvokeEvent(BeforeOnPlacedBlock, toJsonString(jv).data(), &root)) {
		VA reto = original(_this, pPlayer, pBlk, pBlkpos, _bool);
		root.clear();
		InvokeEvent(AfterOnPlacedBlock, toJsonString(jv).data(), &root);
		return reto;
	}
	return NULL;
}

THook(bool,
	MSSYM_B2QUE20destroyBlockInternalB1AA8GameModeB2AAA4AEAAB1UE13NAEBVBlockPosB2AAA1EB1AA1Z,
	void* _this, BlockPos* pBlkpos) {
	auto pPlayer = *reinterpret_cast<Player**>(reinterpret_cast<VA>(_this) + 8);
	auto pBlockSource = *(BlockSource**)(*((__int64*)_this + 1) + 840i64);
	auto pBlk = pBlockSource->getBlock(pBlkpos);
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	jv["blockid"] = pBlk->getLegacyBlock()->getBlockItemID();
	jv["blockname"] = pBlk->getLegacyBlock()->getFullName();
	Json::Value root;
	if (!InvokeEvent(BeforeOnDestroyBlock, toJsonString(jv).data(), &root)) {
		bool ret = original(_this, pBlkpos);
		root.clear();
		InvokeEvent(AfterOnDestroyBlock, toJsonString(jv).data(), &root);
		return ret;
	}
	return false;
}

THook(bool,
	MSSYM_B1QA3useB1AE10ChestBlockB2AAA4UEBAB1UE11NAEAVPlayerB2AAE12AEBVBlockPosB3AAAA1Z,
	void* _this, Player* pPlayer, BlockPos* pBlkpos) {
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	Json::Value root;
	if (!InvokeEvent(BeforeOnStartOpenChest, toJsonString(jv).data(), &root)) {
		bool ret = original(_this, pPlayer, pBlkpos);
		root.clear();
		InvokeEvent(AfterOnStartOpenChest, toJsonString(jv).data(), &root);
		return ret;
	}
	return false;
}

THook(bool,
	MSSYM_B1QA3useB1AE11BarrelBlockB2AAA4UEBAB1UE11NAEAVPlayerB2AAE12AEBVBlockPosB3AAAA1Z,
	void* _this, Player* pPlayer, BlockPos* pBlkpos) {
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	Json::Value root;
	if (!InvokeEvent(BeforeOnStartOpenBarrel, toJsonString(jv).data(), &root)) {
		bool ret = original(_this, pPlayer, pBlkpos);
		root.clear();
		InvokeEvent(AfterOnStartOpenBarrel, toJsonString(jv).data(), &root);
		return ret;
	}
	return false;
}

THook(void,
	MSSYM_B1QA8stopOpenB1AE15ChestBlockActorB2AAE15UEAAXAEAVPlayerB3AAAA1Z,
	void* _this, Player* pPlayer) {
	auto real_this = reinterpret_cast<void*>(reinterpret_cast<VA>(_this) - 248);
	auto pBlkpos = reinterpret_cast<BlockActor*>(real_this)->getPosition();
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	Json::Value root;
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	if (!InvokeEvent(BeforeOnStopOpenChest, toJsonString(jv).data(), &root)) {
		original(_this, pPlayer);
		root.clear();
		InvokeEvent(AfterOnStopOpenChest, toJsonString(jv).data(), &root);
	}
	return;
}

THook(void,
	MSSYM_B1QA8stopOpenB1AE16BarrelBlockActorB2AAE15UEAAXAEAVPlayerB3AAAA1Z,
	void* _this, Player* pPlayer) {
	auto real_this = reinterpret_cast<void*>(reinterpret_cast<VA>(_this) - 248);
	auto pBlkpos = reinterpret_cast<BlockActor*>(real_this)->getPosition();
	Json::Value jv;
	Json::Value root;
	addPlayerInfo(jv, pPlayer);
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	if (!InvokeEvent(BeforeOnStopOpenBarrel, toJsonString(jv).data(), &root)) {
		original(_this, pPlayer);
		root.clear();
		InvokeEvent(AfterOnStopOpenBarrel, toJsonString(jv).data(), &root);
	}
}

THook(void,
	MSSYM_B1QA7setSlotB1AE26LevelContainerManagerModelB2AAE28UEAAXHAEBUContainerItemStackB3AAUA1NB1AA1Z,
	LevelContainerManagerModel* _this, int a2, ContainerItemStack* a3) {
	int slot = a2;
	ContainerItemStack* pItemStack = a3;
	auto nid = pItemStack->getId();
	auto naux = pItemStack->getAuxValue();
	auto nsize = pItemStack->getStackSize();
	auto nname = std::string(pItemStack->getName());
	auto pPlayer = _this->getPlayer();
	VA v3 = *((VA*)_this + 1);				
	BlockSource* bs = *(BlockSource**)(*(VA*)(v3 + 848) + 72i64);
	BlockPos* pBlkpos = (BlockPos*)((char*)_this + 176);
	Block* pBlk = bs->getBlock(pBlkpos);
	Json::Value jv;
	Json::Value root;
	jv["itemid"] = nid;
	jv["itemcount"] = nsize;
	jv["itemname"] = std::string(nname);
	jv["itemaux"] = naux;
	addPlayerInfo(jv, pPlayer);
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	jv["blockid"] = pBlk->getLegacyBlock()->getBlockItemID();
	jv["blockname"] = pBlk->getLegacyBlock()->getFullName();
	jv["slot"] = slot;
	if (!InvokeEvent(BeforeOnSetSlot, toJsonString(jv).data(), &root)) {
		original(_this, slot, pItemStack);
		root.clear();
		InvokeEvent(AfterOnSetSlot, toJsonString(jv).data(), &root);
	}
}

THook(bool,
	MSSYM_B2QUE21playerChangeDimensionB1AA5LevelB2AAA4AEAAB1UE11NPEAVPlayerB2AAE26AEAVChangeDimensionRequestB3AAAA1Z,
	void* _this, Player* pPlayer, void* req) {
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	jv["position"] = jv["XYZ"];
	Json::Value root;
	if (!InvokeEvent(BeforeOnChangeDimension, toJsonString(jv).data(), &root)) {
		bool ret = original(_this, pPlayer, req);
		InvokeEvent(AfterOnChangeDimension, toJsonString(jv).data(), &root);
		return ret;
	}
	return false;
}

static bool checkIsPlayer(void* mp) {
	std::map<std::string, Player*>::iterator iter;
	for (iter = onlinePlayers.begin(); iter != onlinePlayers.end(); iter++) {
		Player* pp = iter->second;
		if (mp == pp)
			return true;
	}
	return false;
}

THook(void,
	MSSYM_B1QA3dieB1AA3MobB2AAE26UEAAXAEBVActorDamageSourceB3AAAA1Z,
	Mob* _this, void* dmsg) {
	auto mob_name = _this->getNameTag();
	if (mob_name.length() != 0) {
		char v72;
		__int64  v2[2];
		v2[0] = (__int64)_this;
		v2[1] = (__int64)dmsg;
		auto v7 = *((__int64*)(v2[0] + 856));
		auto srActid = (__int64*)(*(__int64(__fastcall**)(__int64, char*))(*(__int64*)v2[1] + 56i64))(
			v2[1],
			&v72);
		auto SrAct = SYMCALL(Actor*,
			MSSYM_B1QE11fetchEntityB1AA5LevelB2AAE13QEBAPEAVActorB2AAE14UActorUniqueIDB3AAUA1NB1AA1Z,
			v7, *srActid, 0);
		std::string sr_name = "";
		if (SrAct) {
			sr_name = SrAct->getNameTag();
		}
		Json::Value jv;
		jv["mobname"] = mob_name;
		if (_this->getEntityTypeId() == 1) {
			
			if (checkIsPlayer(_this)) {
				addPlayerInfo(jv, (Player*)_this);
				std::string playertype;				
				SYMCALL(std::string&, MSSYM_MD5_af48b8a1869a49a3fb9a4c12f48d5a68, &playertype, 319);
				jv["mobtype"] = playertype;			
			}
		}
		else
			jv["mobtype"] = _this->getEntityTypeName();
		jv["srcname"] = sr_name;
		Json::Value root;
		if (!InvokeEvent(BeforeOnNamedMobDie, toJsonString(jv).data(), &root)) {
			original(_this, dmsg);
			root.clear();
			InvokeEvent(AfterOnNamedMobDie, toJsonString(jv).data(), &root);
		}
		return;
	}
	original(_this, dmsg);
}

THook(bool,
	MSSYM_B1QA9useItemOnB1AA8GameModeB2AAA4UEAAB1UE14NAEAVItemStackB2AAE12AEBVBlockPosB2AAA9EAEBVVec3B2AAA9PEBVBlockB3AAAA1Z,
	void* _this, ItemStack* item, BlockPos* pBlkpos, unsigned __int8 a4, void* v5, Block* pBlk) {
	auto pPlayer = *reinterpret_cast<Player**>(reinterpret_cast<VA>(_this) + 8);
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	if (playerUseItem[jv["playername"].asCString()] == time(0) + item->getId()) {
		original(_this, item, pBlkpos, a4, v5, pBlk);
		return false;
	}
	playerUseItem[jv["playername"].asCString()] = time(0) + item->getId();
	jv["position"] = toJson(pBlkpos->getPosition()->toJsonString());
	jv["itemid"] = item->getId();
	jv["itemaux"] = item->getAuxValue();
	jv["itemname"] = item->getName();
	Json::Value root;
	if (!InvokeEvent(BeforeOnUseItem, toJsonString(jv).data(), &root)) {
		bool result = original(_this, item, pBlkpos, a4, v5, pBlk);
		root.clear();
		InvokeEvent(AfterOnUseItem, toJsonString(jv).data(), &root);
		return result;
	}
	return false;
}

THook(void, MSSYM_B1QA7respawnB1AA6PlayerB2AAA7UEAAXXZ,
	Player* pPlayer) {
	Json::Value jv;
	addPlayerInfo(jv, pPlayer);
	Json::Value root;
	if (!InvokeEvent(BeforeOnRespawn, toJsonString(jv).data(), &root)) {
		original(pPlayer);
		root.clear();
		InvokeEvent(AfterOnRespawn, toJsonString(jv).data(), &root);
	}
	return;
}

THook(void,
	MSSYM_MD5_ad251f2fd8c27eb22c0c01209e8df83c,
	void* _this, std::string& player_name, std::string& target, std::string& msg, std::string& chat_style) {
	Json::Value jv;
	jv["playername"] = player_name;
	jv["target"] = target;
	jv["msg"] = msg;
	jv["chatstyle"] = chat_style;
	Json::Value root;
	if (!InvokeEvent(BeforeOnChat, toJsonString(jv).data(), &root)) {
		original(_this, player_name, target, msg, chat_style);
		root.clear();
		InvokeEvent(AfterOnChat, toJsonString(jv).data(), &root);
	}
	return;
}

THook(void,
	MSSYM_B1QA6handleB1AE20ServerNetworkHandlerB2AAE26UEAAXAEBVNetworkIdentifierB2AAE14AEBVTextPacketB3AAAA1Z,
	VA _this, VA id, TextPacket* tp) {
	Json::Value jv;
	Player* p = SYMCALL(Player*, MSSYM_B2QUE15getServerPlayerB1AE20ServerNetworkHandlerB2AAE20AEAAPEAVServerPlayerB2AAE21AEBVNetworkIdentifierB2AAA1EB1AA1Z,
		_this, id, *((char*)tp + 16));
	if (p != NULL) {
		addPlayerInfo(jv, p);
	}
	jv["msg"] = tp->toString();
	Json::Value root;
	if (!InvokeEvent(BeforeOnInputText, toJsonString(jv).data(), &root)) {
		original(_this, id, tp);
		root.clear();
		InvokeEvent(AfterOnInputText, toJsonString(jv).data(), &root);
	}
	return;
}

THook(void, MSSYM_B1QE14jumpFromGroundB1AA6PlayerB2AAA7UEAAXXZ, Player* p) {
	Json::Value jv;
	if (p != NULL) {
		addPlayerInfo(jv, p);
	}
	Json::Value root;
	if (!InvokeEvent(BeforeOnPlayerJump, toJsonString(jv).data(), &root)) {
		original(p);
		root.clear();
		InvokeEvent(AfterOnPlayerJump, toJsonString(jv).data(), &root);
	}
	return;
}

THook(VA, MSSYM_B1QE12getAttachPosB1AA6PlayerB2AAA4UEBAB1QA6AVVec3B2AAE15W4ActorLocationB2AAA1MB1AA1Z, Player* _this, VA a1, VA a2, int a3) {
	return original(_this,a1,a2,a3);
}

THook(VA, MSSYM_B1QE14canBeSeenOnMapB1AA6PlayerB2AAA4QEBAB1UA3NXZ, Player* _this) {
	return original(_this);
}

THook(void, MSSYM_B1QE15transformOnFallB1AA9FarmBlockB2AAE20UEBAXAEAVBlockSourceB2AAE12AEBVBlockPosB2AAA9PEAVActorB2AAA1MB1AA1Z, void* _this, BlockSource* a2, BlockPos* a3, Actor* a4, float a5) {
	Json::Value jv;
	jv["blockid"] = a2->getBlock(a3)->getLegacyBlock()->getBlockItemID();
	jv["blockname"] = a2->getBlock(a3)->getLegacyBlock()->getFullName();
	jv["blockpos"] = a3->getPosition()->toJsonString();
	jv["height"] = a5;
	jv["dimensionid"] = a4->getDimensionId();
	jv["entitytype"] = a4->getEntityTypeName();
	jv["entityname"] = a4->getNameTag().length()==0?a4->getTypeName(): a4->getNameTag();
	Json::Value root;
	if (!InvokeEvent(BeforeOnEntityFall, toJsonString(jv).data(), &root)) {
		original(_this,a2,a3,a4,a5);
		root.clear();
		InvokeEvent(AfterOnEntityFall, toJsonString(jv).data(), &root);
	}
}


THook(void,
	MSSYM_B1QA6handleB1AE20ServerNetworkHandlerB2AAE26UEAAXAEBVNetworkIdentifierB2AAE24AEBVCommandRequestPacketB3AAAA1Z,
	VA _this, VA id, CommandRequestPacket* crp) {
	Json::Value jv;
	Player* p = SYMCALL(Player*, MSSYM_B2QUE15getServerPlayerB1AE20ServerNetworkHandlerB2AAE20AEAAPEAVServerPlayerB2AAE21AEBVNetworkIdentifierB2AAA1EB1AA1Z,
		_this, id, *((char*)crp + 16));

	if (p != NULL) {
		addPlayerInfo(jv, p);
	}
	jv["cmd"] = crp->toString();
	if (jv["cmd"] == "/test1") {
		SYMCALL(void, MSSYM_B1QE15changeDimensionB1AE12ServerPlayerB2AAA6UEAAXVB2QDE11AutomaticIDB1AE10VDimensionB2AAA1HB3AAUA1NB1AA1Z, *p, 1);
		return;
	}
	Json::Value root;
	if (!InvokeEvent(BeforeOnInputCommand, toJsonString(jv).data(), &root)) {
		original(_this, id, crp);
		root.clear();
		InvokeEvent(AfterOnInputCommand, toJsonString(jv).data(), &root);
	}
}


THook(Player*,
	MSSYM_B2QUE15createNewPlayerB1AE20ServerNetworkHandlerB2AAE20AEAAAEAVServerPlayerB2AAE21AEBVNetworkIdentifierB2AAE21AEBVConnectionRequestB3AAAA1Z,
	VA a1, VA a2, VA** a3) {
	auto pPlayer = original(a1, a2, a3);
	Json::Value jv;
	jv["playername"] = pPlayer->getNameTag();
	auto uuid = pPlayer->getUuid()->toString();
	jv["uuid"] = uuid;
	jv["xuid"] = std::string(pPlayer->getXuid(p_level));
	onlinePlayers[uuid] = pPlayer;
	Json::Value root;
	
	if (!InvokeEvent(BeforeOnLoadName, toJsonString(jv).data(), &root)) {
		root.clear();
		InvokeEvent(AfterOnLoadName, toJsonString(jv).data(), &root);
	}
	return pPlayer;
}


THook(void,
	MSSYM_B2QUE12onPlayerLeftB1AE20ServerNetworkHandlerB2AAE21AEAAXPEAVServerPlayerB3AAUA1NB1AA1Z,
	VA _this, Player* pPlayer, char v3) {
	Json::Value jv;
	jv["playername"] = pPlayer->getNameTag();
	auto uuid = pPlayer->getUuid()->toString();
	jv["uuid"] = uuid;
	jv["xuid"] = pPlayer->getXuid(p_level);
	onlinePlayers[uuid] = NULL;
	onlinePlayers.erase(uuid);
	playerInventory.erase(pPlayer);
	Json::Value root;
	if (!InvokeEvent(BeforeOnPlayerLeft, toJsonString(jv).data(), &root)) {
		original(_this, pPlayer, v3);
		root.clear();
		InvokeEvent(AfterOnPlayerLeft, toJsonString(jv).data(), &root);
	}
}


THook(void,
	MSSYM_B1QA4moveB1AA6PlayerB2AAE13UEAAXAEBVVec3B3AAAA1Z,
	Player* _this, Vec3* a2) {
	if ((a2->x == 0 || a2->x == -0) && (a2->y == 0 || a2->y == -0) && (a2->z == 0 || a2->z == -0)) {
		original(_this, a2);
		return;
	}
	Json::Value jv;
	addPlayerInfo(jv, _this);
	
	Json::Value root;
	
	if (!InvokeEvent(BeforeOnMove, toJsonString(jv).data(), &root)) {
		original(_this, a2);
		root.clear();
		InvokeEvent(AfterOnMove, toJsonString(jv).data(), &root);
		
	}
	return;

}


THook(bool,
	MSSYM_B1QA6attackB1AA6PlayerB2AAA4UEAAB1UE10NAEAVActorB3AAAA1Z,
	Player* class_this, Actor* pactor) {
	std::string p_player_name = class_this->getNameTag();
	std::string p_actor_name = pactor->getNameTag();
	std::string actor_typename = pactor->getTypeName();
	Vec3* p_position = class_this->getPos();
	Json::Value jv;
	addPlayerInfo(jv, class_this);
	jv["actorname"] = p_actor_name;
	jv["actortype"] = actor_typename;
	jv["position"] = jv["XYZ"];
	std::string strv = toJsonString(jv);
	Json::Value root;
	if (!InvokeEvent(BeforeOnAttack, toJsonString(jv).data(), &root)) {
		bool ret = original(class_this, pactor);
		root.clear();
		InvokeEvent(AfterOnAttack, toJsonString(jv).data(), &root);
		return ret;
	}
	return false;
}









THook(void*, MSSYM_B1QE11triggerRaidB1AE20RaidTriggerComponentB2AAE14AEAAXAEAVActorB3AAAA1Z, void* _this, Actor* a2) {
	Json::Value jv;
	if ((VA)a2 != 0) {
		jv["entity"] = a2->getEntityTypeName();
		jv["entityId"] = a2->getEntityTypeId();
		jv["entityName"] = a2->getNameTag();
		jv["entityPos"] = a2->getPos()->toJsonString();
		jv["dimensionId"] = a2->getDimensionId();
		
	}
	else
	{
		DebugPr("Raid Actor is not entity!!!");
	}
	Json::Value root;
	if (!InvokeEvent(BeforeOnRaidStart, toJsonString(jv).data(), &root)) {
		original(_this, a2);
		root.clear();
		InvokeEvent(AfterOnRaidStart, toJsonString(jv).data(), &root);
	}
	return original(_this, a2);
}














































THook(void*, MSSYM_B2QQA51RaidB2AAA4QEAAB1AA2XZ, void* _this, void* a2) {
	Json::Value jv;
	Json::Value root;
	if (!InvokeEvent(BeforeOnRaidEnd, toJsonString(jv).data(), &root)) {
		original(_this, a2);
		root.clear();
		InvokeEvent(AfterOnRaidEnd, toJsonString(jv).data(), &root);
	}
	return original(_this, a2);
}

THook(void, MSSYM_B1QA7explodeB1AA5LevelB2AAE20QEAAXAEAVBlockSourceB2AAA9PEAVActorB2AAA8AEBVVec3B2AAA1MB1UA4N3M3B1AA1Z, void* _this, BlockSource* a2, Actor* a3, Vec3* a4, float a5, bool a6, bool a7, float a8, bool a9) {
	
	Json::Value jv;
	jv["position"] = a4->toJsonString();
	if ((VA)a3 != 0) {
		jv["entity"] = a3->getEntityTypeName();
		jv["entityId"] = a3->getEntityTypeId();
		jv["dimensionId"] = a3->getDimensionId();
		
	}
	jv["explodePower"] = a5;
	Json::Value root;
	if (!InvokeEvent(BeforeOnExploed, toJsonString(jv).data(), &root)) {
		try
		{
			a5 = root["explodePower"].asInt();
		}
		catch (const std::exception&)
		{

		}
		original(_this, a2, a3, a4, a5, a6, a7, a8, a9);
		root.clear();
		InvokeEvent(AfterOnExploed, toJsonString(jv).data(), &root);
	}
	return;

}


void Wchar_tToString(std::string& szDst, wchar_t* wchar)
{
	wchar_t* wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	char* psText; 
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	szDst = psText;
	delete[]psText;
}

std::string GetUStr(const std::string& str)
{
	std::string showname = str;
	int len = strlen(showname.c_str()) + 1;
	char outch[MAX_PATH];
	WCHAR* wChar = new WCHAR[len];
	wmemset(wChar, 0, len);
	MultiByteToWideChar(CP_UTF8, 0, showname.c_str(), len, wChar, len);
	std::string strRet;
	Wchar_tToString(strRet, wChar);
	delete[] wChar;
	return strRet;
}

static void loadAllCSPlugin(std::string path) {
	std::string pair = path + "\\*.csplugin.dll";
	WIN32_FIND_DATAA ffd;
	HANDLE dfh = FindFirstFileA(pair.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE != dfh) {
		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string fileName = std::string(path + "\\" + ffd.cFileName);
				size_t len = MultiByteToWideChar(CP_ACP, 0, fileName.c_str(), fileName.length(), NULL, 0);
				LPWSTR w_str = new WCHAR[len + 1];
				w_str[len] = L'\0';
				MultiByteToWideChar(CP_ACP, 0, fileName.c_str(), fileName.length(), w_str, len);
				LPCWSTR dllName = w_str;
				const char* f = "OnServerStart";
				typedef char* (*OnServerStart)(int);
				HMODULE hDLL = LoadLibrary(dllName);
				delete dllName;
				if (hDLL == NULL) {
					std::cout << u8"加载插件失败" << std::endl;
				}
				else
				{
					OnServerStart onServeStart = OnServerStart(GetProcAddress(hDLL, f));
					if (onServeStart != NULL)
					{
						std::random_device rd;
						std::mt19937 mt(rd());
						int appId = mt();
						char* ret = (*onServeStart)(appId);
						Json::Reader reader;
						Json::Value root;
						if (reader.parse(ret, root)) {
							try
							{
								PluginInfo p = PluginInfo();
								p.pluginHandle = hDLL;
								p.pluginAuthor = root["pluginAuthor"].asString();
								p.pluginDes = root["pluginDes"].asString();
								p.pluginVer = root["pluginVer"].asString();
								p.pluginName = root["pluginName"].asString();
								plugins[appId] = p;
								std::cout << "[BDSCSRunner]" << p.pluginName << "(" << p.pluginVer << u8")插件已加载" << std::endl;
							}
							catch (const std::exception& ex)
							{
								std::cout << ex.what() << std::endl;
							}
						}
						else
						{
							std::cout << u8"插件启动函数返回有误,无法加载" + fileName << std::endl;
						}
					}
					else
					{
						std::cout << u8"插件缺少启动函数,无法加载" + fileName << std::endl;
					}
				}
			}
		} while (FindNextFileA(dfh, &ffd) != 0);
		FindClose(dfh);
	}
}

static char localpath[MAX_PATH] = { 0 };

static std::string getLocalPath() {
	if (!localpath[0]) {
		GetModuleFileNameA(NULL, localpath, _countof(localpath));
		for (int l = strlen(localpath); l >= 0; l--) {
			if (localpath[l] == '\\') {
				localpath[l] = localpath[l + 1] = localpath[l + 2] = 0;
				break;
			}
		}
	}
	return std::string(localpath);
}

void init() {
	std::cout << u8"[BDSCSRunner]开始加载插件！" << std::endl;
	loadAllCSPlugin(getLocalPath() + "\\CSPlugin");
	std::cout << u8"[BDSCSRunner]插件加载完毕！" << std::endl;
}

void exit() {
	
}

