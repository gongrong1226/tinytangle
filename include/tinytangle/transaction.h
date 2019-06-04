#pragma once
#include <tinytangle/logging.hpp>
#include <tinytangle/sha256.hpp>
#include <json/json.h>
#include <string>
#include <array>
#include <random>
#include <sstream>
#include <tinytangle/keypair.h>

namespace tangle {
	
int64_t get_now_timestamp();

typedef pubkey_t payer_address;
typedef pubkey_t payee_address;
typedef int64_t value;
typedef std::tuple<payer_address, value, payee_address> tx_item_t; //无脚本 负数为收入整数为输出

class Transaction
{
public:

	Transaction();
	//内容发布者，无交易，无接收者，消息内容
	Transaction(const payer_address& payer, const std::string& message, value value = 0, const payee_address& payee = "");
	Transaction(const Transaction& transction);
	Transaction(const Json::Value& json);
	Transaction(Transaction&&) = default;

	Transaction& operator=(const Transaction& transction);
	Transaction& operator=(Transaction&&) = default;

	tx_item_t getTX() const;
	std::string getMessage() const;
	int64_t getTimestamp() const;
	payer_address getPayer() const;
	payee_address getPayee() const; 
	value getValue() const;

	Json::Value item_to_json(const tx_item_t& tx);
	Json::Value to_json();

	//打印交易，待完善
	void print();
	void test();
	//sha256_t getHash() const { return hash_; }

private:
	tx_item_t tx_item_;
	int64_t timestamp_;
	std::string message_;
	//sha256_t hash_;
};




}