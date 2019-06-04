#include <tinytangle/dag.h>

namespace tangle{


/***************class Consensus***************/

void Consensus::PoW(Unit& unit) {
	uint32_t difficulty = unit.getHheader().difficulty;
	log::debug(__FUNCTION__) << "difficulty: " << difficulty;
	// �����ڿ�Ŀ��ֵ,���ֵ�����ѶȾ���Ŀ��ֵ
	uint64_t target = 0xffffffffffffffff / difficulty;
	unit.header_.timestamp = get_now_timestamp();
	// ����Ŀ��ֵ
	for (uint64_t n = 0; ; ++n) {
		//���Ժ�ѡĿ��ֵ
		unit.header_.nonce = n;
		auto&& jv_block = unit.to_json();
		auto&& can = getHash256(jv_block);//���ﻹҪ�ٿ�һ��jv_blockת����String�ж೤
		uint64_t ncan = std::stoull(can.substr(0, 16), 0, 16); //�ض�ǰ16λ��ת��uint64 ����бȽ�

		// �ҵ���
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
	//PoW֮ǰ��û�н���ǩ���ġ�
	root["header"]["signature"] = "";
	//Ҳû��hash
	sha256_t hash = root["header"]["hash"].asString();
	root["header"]["hash"] = "";

	uint32_t difficulty = root["header"]["difficulty"].asUInt();
	uint64_t target = 0xffffffffffffffff / difficulty;

	auto&& can = getHash256(root);
	uint64_t ncan = std::stoull(can.substr(0, 16), 0, 16); //�ض�ǰ16λ��ת��uint64 ����бȽ�
	if (ncan < target  && hash == can) {
		return true;
	}
	return false;
}


}