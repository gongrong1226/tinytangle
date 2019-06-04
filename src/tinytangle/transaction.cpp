#include <tinytangle/transaction.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace tangle {

int64_t get_now_timestamp() {
	boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
	boost::posix_time::time_duration time_from_epoch = boost::posix_time::second_clock::universal_time() - epoch;
	return time_from_epoch.total_seconds();
}

Transaction::Transaction(){
}

Transaction::Transaction(const payer_address& payer, const std::string& message, 
							value value, const payee_address& payee){
	tx_item_ = std::make_tuple(payer, value, payee);
	message_ = message;
	timestamp_ = get_now_timestamp();
}

Transaction::Transaction(const Transaction& transction) {
	tx_item_ = transction.getTX();
	message_ = transction.getMessage();
	timestamp_ = transction.getTimestamp();
}

Transaction::Transaction(const Json::Value& json) {
	if (json["tx_item"].type() != Json::nullValue
		&& json["tx_item"]["payer"].type() != Json::nullValue
		&& json["tx_item"]["value"].type() != Json::nullValue
		&& json["tx_item"]["payee"].type() != Json::nullValue
		&& json["message"].type() != Json::nullValue
		&& json["timestamp"].type() != Json::nullValue)
	{
		payer_address payer, payee, message;
		uint64_t value;
		int64_t timestamp_;
		payer = json["tx_item"]["payer"].asString();
		payee = json["tx_item"]["payee"].asString();
		value = json["tx_item"]["value"].asInt64();

		tx_item_ = std::make_tuple(payer, value, payee);
		message_ = json["message"].asString();
		timestamp_ = json["timestamp"].asInt64();
	}
}

Transaction& Transaction::operator=(const Transaction& transction) {
	tx_item_ = transction.getTX();
	message_ = transction.getMessage();
	timestamp_ = transction.getTimestamp();
	return *this;
}

tx_item_t Transaction::getTX(void) const {
	return tx_item_;
}

std::string Transaction::getMessage(void) const {
	return message_;
}

int64_t Transaction::getTimestamp(void) const {
	return timestamp_;
}

payer_address Transaction::getPayer(void) const {
	return std::get<0>(tx_item_);
}
payee_address Transaction::getPayee(void) const {
	return std::get<2>(tx_item_);
}
value Transaction::getValue(void) const {
	return std::get<1>(tx_item_);
}

/*Transaction::Transaction(address_t& address) {
	// coinbase Transaction: TODO
	tx_ = std::make_tuple("00000000000000000000000000000000", 0, "0ffffffffffffff");
	to_json();
}*/

Json::Value Transaction::item_to_json(const tx_item_t& tx) {
	Json::Value root;
	root["payer"] = std::get<0>(tx);
	root["value"] = std::get<1>(tx);
	root["payee"] = std::get<2>(tx);
	return root;
}

Json::Value Transaction::to_json() {
	Json::Value root;
	root["tx_item"] = item_to_json(tx_item_);
	root["message"] = message_;
	root["timestamp"] = timestamp_;
	return root;
}

void Transaction::print(){
	std::cout << "[payer]" << std::get<0>(tx_item_) << "\n"
			  << "[value]" << std::get<1>(tx_item_) << "\n"
			  << "[payee]" << std::get<2>(tx_item_) << "\n"
			  << "[message]" << message_ << "\n"
			  << "[timestamp]" << timestamp_ << std::endl;

}

}