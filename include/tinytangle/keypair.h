#pragma once

#include <tinytangle/logging.hpp>
// Encryption Algorithm from cryptopp
#include <cryptopp/rsa.h> 
#include <cryptopp/base64.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1 //ignore #warning from comipler for MD5
#include <cryptopp/md5.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pssr.h>
#include <cryptopp/whrlpool.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/queue.h>
#include <cryptopp/integer.h>
using CryptoPP::ByteQueue;
#include <cryptopp/files.h>
using CryptoPP::FileSource;
using CryptoPP::FileSink;
#include <json/json.h>
#include <tinytangle/sha256.hpp>

namespace tangle {
	
class test {
public: 
	static std::string globalStr;
	static void setStr(std::string k) {
		globalStr = k;
	}
};
// ---------------------------- typedef ----------------------------
typedef std::string sha256_t;
typedef std::string md5_t;
typedef std::string address_t;
typedef std::string pubkey_t;
typedef std::string script_sign_t;
typedef std::string script_pubkey_t;
typedef std::string data_t;
typedef CryptoPP::RSA::PublicKey public_key_t;
typedef CryptoPP::RSA::PrivateKey private_key_t;


void addr2String(std::string &addr, std::string &dest);

bool jsonToString(const Json::Value &json, std::string& str);
//使用sha256得到hash
sha256_t getHash256(Json::Value jv);
//使用md5得到信息摘要
md5_t to_md5(const std::string& message);
//从公钥推导地址
address_t key_to_address(const public_key_t& public_key);
// 从私钥推导地址
address_t key_to_address(const private_key_t& private_key);

//采用RSA非对称加密算法，生成RSA密钥对，可升级为ECC椭圆曲线加密，待完善
class KeyPair
{
public:
	// new key pair
	KeyPair();
	// Copy
	KeyPair(const KeyPair& rk);
	// =
	KeyPair& operator=(const KeyPair& rk);

	// 从base64直接构造RSA私钥(密钥本身不可打印，base64可以）
	KeyPair(std::string& encoded_prik);

	// TODO
	KeyPair(KeyPair&&) = default;
	KeyPair& operator=(KeyPair&&) = default;

	void print();
	void test();

	//从private_key得到public_key，再将公钥md5摘要(暂定直接pubKey)
	pubkey_t address() const;

	private_key_t getPrvKey() const;

	// 构造base64的可打印字符串密钥对
	std::pair<std::string, std::string>	encode_pair() const;

	// 格式化为JSON
	Json::Value to_json() const;

	//做md5摘要再做签名
	static std::string sinature(const std::string& unitStr, private_key_t privateKey);
	static bool verifySignature(Json::Value root);

	static bool AddressToKey(std::string pubkey_string, public_key_t& public_key);

public:
	CryptoPP::AutoSeededRandomPool rng;

private:
	private_key_t private_key_;
};


}