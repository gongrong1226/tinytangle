#include <tinytangle/unit.h>

namespace tangle {

Unit::Unit(){
}

Unit::Unit(const Unit& unit_) {
	header_ = unit_.getHheader();
	tx_ = unit_.getTransaction();
}

Unit::Unit(const Json::Value& json) {
	if (json["header"]["nonce"].type() != Json::nullValue
			&&json["header"]["selfWeight"].type() != Json::nullValue
			&&json["header"]["timestamp"].type() != Json::nullValue
			&&json["header"]["difficulty"].type() != Json::nullValue
			&&json["header"]["signature"].type() != Json::nullValue
			&&json["header"]["hash"].type() != Json::nullValue
			&&json["header"]["tipsHash"][0].type() != Json::nullValue
			&&json["header"]["tipsHash"][1].type() != Json::nullValue)
	{
		header_.nonce = json["header"]["nonce"].asUInt();
		header_.selfWeight = json["header"]["selfWeight"].asUInt();
		header_.timestamp = json["header"]["timestamp"].asInt64();
		header_.difficulty = json["header"]["difficulty"].asUInt();
		header_ .signature= json["header"]["signature"].asString();
		header_.hash = json["header"]["hash"].asString();
		header_.tipsHash[0] = json["header"]["tipsHash"][0].asString();
		header_.tipsHash[1] = json["header"]["tipsHash"][1].asString();

		Transaction temp(json["tx"]);
		tx_ = temp;
	}
}


Unit& Unit::operator=(const Unit& rb) {
	header_ = rb.getHheader();
	tx_ = rb.getTransaction();
	return *this;
}

const Transaction& Unit::getTransaction() const{
	return tx_; 
}

const unitHeader& Unit::getHheader() const{
	return header_;
}

sha256_t Unit::getHash() const{ 
	return header_.hash; 
}

void Unit::setup(const Transaction& tx) {
	tx_ = tx; 
}

Json::Value Unit::to_json() {
	Json::Value root;
	Json::Value uheader;

	uheader["nonce"] = header_.nonce;
	uheader["selfWeight"] = header_.selfWeight;
	uheader["timestamp"] = header_.timestamp;
	uheader["difficulty"] = header_.difficulty;
	uheader["hash"] = header_.hash;
	uheader["signature"] = header_.signature;
	uheader["tipsHash"].append(header_.tipsHash[0]);
	uheader["tipsHash"].append(header_.tipsHash[1]);

	root["header"] = uheader;
	root["tx"] = tx_.to_json();
	return root;
}

std::string Unit::to_string() {
	auto&& j = to_json();
	return j.toStyledString();
}

std::string Unit::to_string(const Json::Value json) {
	return json.toStyledString();
}

void Unit::signature(private_key_t privateKey) {
	//set siganature
	std::string &&str = to_string();
	header_.signature = KeyPair::sinature(str, privateKey);
}

bool Unit::verify() {
	Json::Value &&root = to_json();
	return KeyPair::verifySignature(root);
}


}