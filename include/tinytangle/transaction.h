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
typedef std::tuple<payer_address, value, payee_address> tx_item_t; //�޽ű� ����Ϊ��������Ϊ���

class Transaction
{
public:

	Transaction();
	//���ݷ����ߣ��޽��ף��޽����ߣ���Ϣ����
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

	//��ӡ���ף�������
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