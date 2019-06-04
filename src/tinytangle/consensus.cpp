#include <tinytangle/dag.h>

namespace tangle{


/***************class Consensus***************/

void Consensus::PoW(Unit& unit) {
	uint32_t difficulty = unit.getHheader().difficulty;
	log::debug(__FUNCTION__) << "difficulty: " << difficulty;
	// 计算挖矿目标值,最大值除以难度就是目标值
	uint64_t target = 0xffffffffffffffff / difficulty;
	unit.header_.timestamp = get_now_timestamp();
	// 计算目标值
	for (uint64_t n = 0; ; ++n) {
		//尝试候选目标值
		unit.header_.nonce = n;
		auto&& jv_block = unit.to_json();
		auto&& can = getHash256(jv_block);//这里还要再看一下jv_block转换的String有多长
		uint64_t ncan = std::stoull(can.substr(0, 16), 0, 16); //截断前16位，转换uint64 后进行比较

		// 找到了
		if (ncan < target) {
			unit.header_.hash = can;
			//log::info("consensus") << "target:" << ncan;
			//log::info("consensus") << "new block :" << can;
			//log::info(__FUNCTION__) << "new unit: \n" << unit.to_json().toStyledString();
			//log::info(__FUNCTION__) << "get new unit";
			return;
		}
	}
}

bool Consensus::verifyPoW(Json::Value root) {
	//PoW之前是没有进行签名的。
	root["header"]["signature"] = "";
	//也没有hash
	sha256_t hash = root["header"]["hash"].asString();
	root["header"]["hash"] = "";

	uint32_t difficulty = root["header"]["difficulty"].asUInt();
	uint64_t target = 0xffffffffffffffff / difficulty;

	auto&& can = getHash256(root);
	uint64_t ncan = std::stoull(can.substr(0, 16), 0, 16); //截断前16位，转换uint64 后进行比较
	if (ncan < target  && hash == can) {
		return true;
	}
	return false;
}


}