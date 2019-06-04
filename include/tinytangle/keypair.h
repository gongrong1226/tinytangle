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
//ʹ��sha256�õ�hash
sha256_t getHash256(Json::Value jv);
//ʹ��md5�õ���ϢժҪ
md5_t to_md5(const std::string& message);
//�ӹ�Կ�Ƶ���ַ
address_t key_to_address(const public_key_t& public_key);
// ��˽Կ�Ƶ���ַ
address_t key_to_address(const private_key_t& private_key);

//����RSA�ǶԳƼ����㷨������RSA��Կ�ԣ�������ΪECC��Բ���߼��ܣ�������
class KeyPair
{
public:
	// new key pair
	KeyPair();
	// Copy
	KeyPair(const KeyPair& rk);
	// =
	KeyPair& operator=(const KeyPair& rk);

	// ��base64ֱ�ӹ���RSA˽Կ(��Կ�����ɴ�ӡ��base64���ԣ�
	KeyPair(std::string& encoded_prik);

	// TODO
	KeyPair(KeyPair&&) = default;
	KeyPair& operator=(KeyPair&&) = default;

	void print();
	void test();

	//��private_key�õ�public_key���ٽ���Կmd5ժҪ(�ݶ�ֱ��pubKey)
	pubkey_t address() const;

	private_key_t getPrvKey() const;

	// ����base64�Ŀɴ�ӡ�ַ�����Կ��
	std::pair<std::string, std::string>	encode_pair() const;

	// ��ʽ��ΪJSON
	Json::Value to_json() const;

	//��md5ժҪ����ǩ��
	static std::string sinature(const std::string& unitStr, private_key_t privateKey);
	static bool verifySignature(Json::Value root);

	static bool AddressToKey(std::string pubkey_string, public_key_t& public_key);

public:
	CryptoPP::AutoSeededRandomPool rng;

private:
	private_key_t private_key_;
};


}