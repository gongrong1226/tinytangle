#pragma once
#include <tinytangle/keypair.h>
#include <tinytangle/transaction.h>
#include <tinytangle/unit.h>
#include <tinytangle/database.h>
#include <tinytangle/consensus.h>
#include <mutex>
#include <condition_variable>
#include <threadpool.hpp>

namespace tangle {
	using namespace std;
class Dag{
	const int DIFFICULTY = 0x3FFFF; //4��0
	//const int DIFFICULTY = 0xF; //1��0
	//TODO  change vector to mutilmap
	typedef std::vector<Unit> tipsPool_t;  
public:
	Dag(int id);
	Dag(const Dag&) = default;
	Dag(Dag&&) = default;
	Dag& operator=(Dag&&) = default;
	Dag& operator=(const Dag&) = default;
	~Dag() {
		stopProdUnits();
	}
	void init();
	void startProdUnits(const string &msg) {		is_producing_ = true;		thread([&]() {			while (is_producing_) {				thread_pool_.wait_empty();				pushInfo(msg);/////////////			}		}).detach();	}	void stopProdUnits(void) {		is_producing_ = false;	}	void mineOnce(const string &msg) {		pushInfo(msg);	}	void getNewKeyPair(KeyPair& newKey);
	//���ݽ��ס�˽Կ���ɼ�˲��������
	bool creatUnit(Unit& newUnit, const Transaction& tx, const private_key_t& privateKey);

	//��ȡ��������hash��timastamp
	bool getLastUnit(int64_t& timestamp, sha256_t& hash);
	uint64_t getCount();

	//�ڵ��ʼ��ͬ��ʱ�õ�
	void getAllUnit(Json::Value &root);
	void pushAllUnit(const Json::Value& root);

	void pushDatabaseUnit(const Json::Value &root);
	void pushTipsUnit(const Json::Value &root);

	//��ȡָ�����׵�Ԫ
	bool getUnit(sha256_t hash, Unit& dest);

	/**
	*@brief ��JsonתΪUnit������֤
	*@param root ������Unit Json����
	*@return	true ��Ԫ�Ϸ����ѷ���DAG��˻�������
	*		false ��Ԫ���Ϸ�
	*/
	bool pushUnit(const Json::Value& root);

	void eraseTip(const Unit& newUnit);

	//���ѡ�񣬲�����ѡ��˷������ݿ�
	bool selectTips(sha256_t(&tips)[2]); //��Դ��������
	
	//��ѯĿ���ַ���
	bool getBalance(const address_t& encodePubKey, value& balance);

	// ��ȡ��ǰ����Id
	int getId() { return id_; }

	void setDifficulty(uint32_t difficulty);
	uint32_t getDifficulty(void) const;
	void setVersion(const std::string& version);
	std::string getVersion(void) const;
	void pushInfo(const string& str);	void setBroadcastHandler(std::function<void(const Json::Value&)> fun) {		broadcastUnit = fun;	}private:
	void pushTip(const Unit& newUnit);	//ֻ������֤�����Ƿ��ͻ	bool verifyTip(const Unit& tip);
	bool getRandTip(uint32_t& index);

private:
	int id_;
	uint64_t unit_quantity_ = 0;

	std::function<void(const Json::Value& )> broadcastUnit;

	threadpool::fixed_thread_pool thread_pool_;

	DagDatabase dag_db_;
	std::mutex m_dag_db_lock;

	KeyPairDatabase key_db_;

	KeyPair myKey_;

	tipsPool_t tipsPool_;
	std::mutex m_tips_pool_lock;
	std::condition_variable m_tips_pool_cond;

	uint32_t difficulty_;
	std::string version_;

	bool is_producing_;
};




}