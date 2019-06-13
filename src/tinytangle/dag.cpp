#include <tinytangle/dag.h>

namespace tangle {


Dag::Dag(int id):id_(id), thread_pool_(1,0){
	//dag_db_.create_genesis_block;
	difficulty_ = DIFFICULTY;
	version_ = "v1.0";
}

static Unit creatGenesisUnit() {
	Unit genesisUnit;
	const address_t genesisAddr = "0000000000000000000000000000000000000000000000000000000000000000";
	Transaction tx(genesisAddr, "Genesis Unit");

	genesisUnit.header_.tipsHash[0] = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.tipsHash[1] = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.signature = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.timestamp = get_now_timestamp();
	genesisUnit.header_.nonce = 0;
	genesisUnit.header_.difficulty = 0;
	genesisUnit.header_.selfWeight = 0;
	genesisUnit.header_.hash = getHash256(genesisUnit.to_json());
	genesisUnit.setup(tx);
	return genesisUnit;
}

void Dag::init(void) {
	if (dag_db_.getCount() == 0 && tipsPool_.size() == 0) {
		lock_guard<mutex> guard(m_tips_pool_lock);
		Unit genesisUnit = creatGenesisUnit();
		tipsPool_.push_back(genesisUnit);
	}

	//����Ĳ��Ǻܺ�
	if (key_db_.getCount() == 0) {
		getNewKeyPair(myKey_);
	}
	else {
		myKey_ = key_db_.getKeyPair();
	}

	thread_pool_.start();
}
void Dag::getNewKeyPair(KeyPair& newKey) {
	newKey = key_db_.getNewKeyPair();
	//��Ӧ�öԹ�Կ��ַ���й㲥
	//broadcast(pubKey);
}

static uint32_t calcSelfWeight(uint32_t difficulty) {
	int weight = 0;
	while (difficulty / 0xF != 0) {
		difficulty /= 0xF;
		weight++;
	}
	return weight;
}

bool Dag::creatUnit(Unit& newUnit, const Transaction& tx, const private_key_t& privateKey) {
	newUnit.setup(tx);
	newUnit.header_.difficulty = getDifficulty();
	newUnit.header_.selfWeight = calcSelfWeight(newUnit.header_.difficulty);
	newUnit.header_.version = getVersion();
	if (!selectTips(newUnit.header_.tipsHash)) {
		//��˻����������ںϷ�����
		return false;
	}
	//nonce & timestamp
	Consensus::PoW(newUnit);
	log::info(__FUNCTION__) << "pow ok";

	//signature;
	std::string str = newUnit.to_string();
	std::string signature = KeyPair::sinature(str, privateKey);
	newUnit.header_.signature = signature;
	log::info(__FUNCTION__) << "signature ok";

	//�����˳�
	pushTip(newUnit);
	log::info(__FUNCTION__) << "pushTip ok";

	//����㲥����

	//��ʱ5S�ٷ��ͽ���
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	broadcastUnit(newUnit.to_json());
	log::info(__FUNCTION__) << "broadcastUnit ok";


	return true;
}


//����������ı��ģ�unit)ֻ���м��ѡ��ǩ����PoW��Ҳ��ͨ����֤��������
//�����᲻��������ѡ��˵�Ȩ�أ�����selectTip����Ҫ
bool Dag::pushUnit(const Json::Value& root) {
	Json::Value js = root;
	sha256_t hash = js["header"]["hash"].asString();

	//����Ƿ������dag database��
	Unit testUnit;
	if (dag_db_.getUnit(hash, testUnit)) {
		return false;
	}

	{//����Ƿ�����ڼ�˽��׳���
		lock_guard<mutex> guard(m_tips_pool_lock);
		for (Unit u : tipsPool_) {
			if (u.getHash() == hash) {
				return false;
			}
		}
	}

	//��֤ǩ��
	if (!KeyPair::verifySignature(js)) {
		return false;
	}

	//��֤PoW
	if (!Consensus::verifyPoW(js)) {
		return false;
	}

	Unit newUnit(root);
	//����Ӧ��˷������ݿ�
	sha256_t tip1 = newUnit.getHheader().tipsHash[0];
	sha256_t tip2 = newUnit.getHheader().tipsHash[1];

	{
		std::lock_guard<std::mutex> guard(m_tips_pool_lock);
		for (auto i = tipsPool_.begin(); i < tipsPool_.end(); i++) {
			if ((*i).getHash() == tip1 || (*i).getHash() == tip2) {
				dag_db_.push((*i));
				////////////////�ش�BUG
				//�������ݿ⵫û�в������
				tipsPool_.erase(i);
				if (tip1 == tip2)
					break;
				
			}
		}
	}

	//�����յ��ĵ�Ԫ�����˻�����
	pushTip(newUnit);
	return true;
}

void Dag::pushTip(const Unit& newUnit) {
	std::lock_guard<std::mutex> guard(m_tips_pool_lock);
	tipsPool_.push_back(newUnit);	m_tips_pool_cond.notify_one();//�����½��׵�ʱ����Ҫѡ����}//TODObool Dag::verifyTip(const Unit& tip) {
	if (tip.getHheader().tipsHash[0] == "0000000000000000000000000000000000000000000000000000000000000000") {
		return true;
	}

	Unit testUnit;
	if (dag_db_.getUnit(tip.getHash(), testUnit)) {
		return false;
	}
	return true;
}

//TODO ���ڸ�ΪMCMC
bool Dag::getRandTip(uint32_t& index) {
	uint32_t size = tipsPool_.size();
	if (size < 1)
		return false; //û�м�˽�����
	std::vector<int> rand;
	for (uint32_t i = 0; i < size; i++)
		rand.push_back(i);
	random_shuffle(rand.begin(), rand.end());
	std::string  k;
	index = rand[0];
	return true;
}

bool Dag::selectTips(sha256_t (&tipsHash)[2]) { //�����ʱ��Դ�������⣿
	uint32_t rand = 0;
	int tipCount = 0;
	tipsHash[0] = ""; //initialize
	tipsHash[1] = "";
	std::unique_lock<std::mutex> lock(m_tips_pool_lock);
	if (tipsPool_.size() < 1) {
		tangle::log::debug(__FUNCTION__) << "waiting for tips";
		m_tips_pool_cond.wait(lock);
	}
	while (tipCount < 2 && getRandTip(rand) ) {
		if (verifyTip(tipsPool_[rand])) {
			//��˺Ϸ�
			tipsHash[tipCount] = tipsPool_[rand].getHash();
			{	
				std::lock_guard<std::mutex> guard(m_dag_db_lock);
				dag_db_.push(tipsPool_[rand]); 
			}
			++tipCount;
		}
		else {
		}
		//�����Ƿ�Ϸ���Ӧ�ò���
		auto begin = tipsPool_.begin();
		tipsPool_.erase(begin + rand);
	}
	lock.unlock();
	if (tipsHash[0] == "")	//��˻����������ںϷ�����
		return false;
	if (tipsHash[1] == "")	//ֻ����һ�����
		tipsHash[1] = tipsHash[0];
	return true;
}

//��ȡ��������hash��timastamp
bool Dag::getLastUnit(int64_t& timestamp, sha256_t& hash) {
	return dag_db_.getLastUnit(timestamp, hash);
}

void Dag::getAllUnit(Json::Value &root) {
	dag_db_.getAllUnit(root["database"]);
	lock_guard<mutex> guard(m_tips_pool_lock);
	int n = 0;
	for (auto unit : tipsPool_) {
		root["tips"][n++] = unit.to_json();
	}
}

void Dag::pushAllUnit(const Json::Value& root) {	if (root.empty()) {		//�����̳߳أ����������Unit		thread_pool_.start();		return;	}	uint64_t newSize = root["database"].size();	uint64_t oldSize = getCount();	if (newSize > oldSize) {		dag_db_.pushAllUnit(root["database"]);	}	lock_guard<mutex> guard(m_tips_pool_lock);	newSize = root["tips"].size();	oldSize = tipsPool_.size();	if (newSize > oldSize) {		for (int i = 0; i < root["tips"].size(); ++i) {			Unit m_unit(root["tips"][i]);			tipsPool_.push_back(m_unit);		}	}	//�����̳߳أ����������Unit	thread_pool_.start();}void Dag::pushInfo(const string& str){
	Unit newUnit;
	Transaction tx(myKey_.address(), str);
	std::function<void()> fun = bind(&Dag::creatUnit, this, newUnit, tx, myKey_.getPrvKey());
	thread_pool_.execute(fun);
}

uint64_t Dag::getCount() {
	return (dag_db_.getCount()+tipsPool_.size());
}

bool Dag::getUnit(sha256_t hash, Unit& dest) {
	return dag_db_.getUnit(hash, dest);
}

bool Dag::getBalance(const address_t& encodePubKey, value& balance) {
	return key_db_.getBalance(encodePubKey, balance);
}

void Dag::setDifficulty(uint32_t difficulty) {
	difficulty_ = difficulty;
}

uint32_t Dag::getDifficulty(void)const{
	return difficulty_;
}

void Dag::setVersion(const std::string& version) {
	version_ = version;
}

std::string Dag::getVersion(void) const {
	return version_;
}


}