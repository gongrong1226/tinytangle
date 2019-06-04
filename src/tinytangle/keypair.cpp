#include <tinytangle/keypair.h>

namespace tangle {


//DERencode -> public_key -> md5
void addr2String(std::string &addr, std::string &dest) {
	public_key_t public_key;
	KeyPair::AddressToKey(addr, public_key);

	std::string x509_pubk; //binary pubk as x.509
	CryptoPP::StringSink ss(x509_pubk);
	public_key.Save(ss);
	//int length = x509_pubk.size(); //160 Bytes
	log::debug(__FUNCTION__) << x509_pubk;
	// 进行一次MD5作为地址
	dest = to_md5(x509_pubk);
}

sha256_t getHash256(Json::Value jv){
	Json::StreamWriterBuilder builder;
	std::ostringstream oss;
	//toStyleString????
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(jv, &oss);
	//std::cout << oss.str() << std::endl;
	return sha256(oss.str());
}

bool jsonToString(const Json::Value &json, std::string& str) {
	Json::StreamWriterBuilder builder;
	std::ostringstream oss;
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(json, &oss);
	str = oss.str();
	return true;
}

//使用md5得到hash
md5_t to_md5(const std::string& message) {
	uint8_t digest[CryptoPP::Weak::MD5::DIGESTSIZE];

	CryptoPP::Weak::MD5 hash;
	//得到信息摘要
	//https://www.cryptopp.com/wiki/Hash_Functions#The_MD5_algorithm
	hash.CalculateDigest(digest, (const uint8_t*)message.c_str(), message.length());

	CryptoPP::HexEncoder encoder;
	std::string output;

	encoder.Attach(new CryptoPP::StringSink(output));
	encoder.Put(digest, sizeof(digest));
	encoder.MessageEnd();

	return output;
}

// 从公钥推导地址
address_t key_to_address(const public_key_t& public_key) {

	//https://blog.csdn.net/yo_joky/article/details/47041385
	ByteQueue queue;
	std::ostringstream  osstring;
	public_key.DEREncodePublicKey(queue);
	FileSink file(osstring);
	queue.CopyTo(file);
	file.MessageEnd();
	std::string pubkey_string = osstring.str();
	//int lengt1h = qs.size(); //138 Bytes
	//log::debug(__FUNCTION__) << pubkey_string;

	/*
	//https://www.cryptopp.com/wiki/BERDecode
	std::string pubkey_string;
	//CryptoPP::StringSource pubkey_ss(pubkey_string, true, new CryptoPP::Base64Encoder);
	CryptoPP::HexEncoder encoder;
	encoder.Attach(new CryptoPP::StringSink(pubkey_string));
	public_key.Save(encoder);
	int length = pubkey_string.size();//320 Bytes
	log::debug(__FUNCTION__) << pubkey_string ;
	//暂不做MD5，直接返回pubkey

	CryptoPP::HexDecoder decoder;
	//decoder.Put((byte*)pubkey_string.c_str(), pubkey_string.size());
	decoder.MessageEnd();
	public_key_t pktemp;
	pktemp.Load(decoder);
	*/

	/*
	//首先获取标准格式的公钥
	std::string x509_pubk; //binary pubk as x.509
	CryptoPP::StringSink ss(x509_pubk);
	public_key.Save(ss);
	int length2 = x509_pubk.size(); //160 Bytes
	log::debug(__FUNCTION__) << x509_pubk;
	// 进行一次MD5作为地址
	//return to_md5(x509_pubk);
	*/

	return pubkey_string;
}

// 从私钥得到公钥地址，再将公钥转换md5(暂定为直接pubKey）
address_t key_to_address(const private_key_t& private_key) {
	public_key_t public_key(private_key);
	return key_to_address(public_key);
}

KeyPair::KeyPair(){
	private_key_.GenerateRandomWithKeySize(rng, 1024);
}

KeyPair::KeyPair(const KeyPair& rk) {
	private_key_ = rk.getPrvKey();
}

KeyPair::KeyPair(std::string& encoded_prik) {

	//log::debug("KeyPair-in") << encoded_prik;
	// decode base64 into private key
	CryptoPP::StringSource prik_ss(encoded_prik, true, new CryptoPP::Base64Decoder());
	private_key_.BERDecode(prik_ss);
	//log::debug("KeyPair-out") << to_json();
}

KeyPair& KeyPair::operator=(const KeyPair& rk){
	private_key_ = rk.getPrvKey();
	return *this;
}

void KeyPair::print(){
	log::info("KeyPair") << to_json();
}

pubkey_t KeyPair::address() const {
	return key_to_address(private_key_);
}

std::pair<std::string, std::string> KeyPair::encode_pair() const {
	// get public key
	public_key_t pubKey(private_key_);
	// encode with base64
	std::string encoded_prik, encoded_pubk;
	CryptoPP::Base64Encoder prik_slink(new CryptoPP::StringSink(encoded_prik), false);//false for no '\n'
	CryptoPP::Base64Encoder pubk_slink(new CryptoPP::StringSink(encoded_pubk), false);
	private_key_.DEREncode(prik_slink);
	pubKey.DEREncode(pubk_slink);
	prik_slink.MessageEnd();//base64 编码补足=  844
	pubk_slink.MessageEnd();//216
	//log::debug("key_pair-0")<<encoded_prik;
	return std::make_pair(encoded_prik, encoded_pubk);
}

private_key_t KeyPair::getPrvKey() const { 
	return private_key_;
}

Json::Value KeyPair::to_json() const {
	Json::Value root;
	auto&& keypair = encode_pair();
	root["address"] = address();//私钥->公钥->md5(暂定string)
	root["public_key"] = keypair.second;
	root["private_key"] = keypair.first;
	return root;
}

bool KeyPair::AddressToKey(std::string pubkey_string, public_key_t& public_key) {
	ByteQueue decodeQueue;
	std::istringstream  instring(pubkey_string);
	//log::debug(__FUNCTION__) << instring.str();
	FileSource defile(instring, true /*pumpAll*/);
	defile.TransferTo(decodeQueue);
	decodeQueue.MessageEnd();
	try
	{
		public_key.BERDecodePublicKey(decodeQueue, false /*optParams*/, decodeQueue.MaxRetrievable());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
	return true;
}

std::string KeyPair::sinature(const std::string& unitStr, private_key_t privateKey) {
	using CryptoPP::StringSource;
	using CryptoPP::SignerFilter;
	using CryptoPP::StringSink;
	using CryptoPP::PSSR;
	using CryptoPP::PSS;
	using CryptoPP::InvertibleRSAFunction;
	using CryptoPP::RSASS;
	using CryptoPP::RSA;
	using CryptoPP::SHA256;
	CryptoPP::AutoSeededRandomPool rng;
	InvertibleRSAFunction parameters;
	parameters.GenerateRandomWithKeySize(rng, 1024);

	//get siganature 
	//www.cryptopp.com/wiki/RSA_Signature_Schemes#RSA_Signature_Scheme_with_Appendix_.28Filters.29
	RSASS<PSS, SHA256>::Signer signer(privateKey);
	//std::string &&md = to_string();
	//md5_t str = to_md5(str);
	std::string str = to_md5(unitStr);
	std::string signature;
	StringSource ss1(str, true,
		new SignerFilter(rng, signer,
			new CryptoPP::StringSink(signature)
		) // SignerFilter
	); // StringSource

	return signature;
}


bool KeyPair::verifySignature(Json::Value root) {
	using CryptoPP::StringSource;
	using CryptoPP::StringSink;
	using CryptoPP::PSS;
	using CryptoPP::RSASS;
	using CryptoPP::SHA256;

	//get sinature then reset the sinature
	std::string getSignature = root["header"]["signature"].asString();
	std::string tempstr = "";
	root["header"]["signature"] = tempstr;
	std::string jsonStr = root.toStyledString();

	//get public key of the payer
	std::string pubKeyStr = root["tx"]["tx_item"]["payer"].asString();
	public_key_t publicKey;
	if (!KeyPair::AddressToKey(pubKeyStr, publicKey)) {
		return false;
	}

	//verify
	md5_t md5 = to_md5(jsonStr);
	RSASS<PSS, SHA256>::Verifier verifier(publicKey);
	std::string recovered;
	try
	{
		StringSource ss2(md5 + getSignature, true,
			new CryptoPP::SignatureVerificationFilter(
				verifier,
				new StringSink(recovered),
				CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION |
				CryptoPP::SignatureVerificationFilter::PUT_MESSAGE
			) // SignatureVerificationFilter
		); // StringSource
	}
	catch (CryptoPP::Exception&e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	return true;
}

}