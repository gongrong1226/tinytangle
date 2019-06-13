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

	//处理的不是很好
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
	//还应该对公钥地址进行广播
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
		//尖端缓存区不存在合法交易
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

	//放入尖端池
	pushTip(newUnit);
	log::info(__FUNCTION__) << "pushTip ok";

	//网络广播交易

	//延时5S再发送交易
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	broadcastUnit(newUnit.to_json());
	log::info(__FUNCTION__) << "broadcastUnit ok";


	return true;
}


//如果传过来的报文（unit)只含有尖端选择、签名和PoW，也会通过验证并放入尖端
//这样会不断增加所选尖端的权重，所以selectTip很重要
bool Dag::pushUnit(const Json::Value& root) {
	Json::Value js = root;
	sha256_t hash = js["header"]["hash"].asString();

	//检查是否存在与dag database中
	Unit testUnit;
	if (dag_db_.getUnit(hash, testUnit)) {
		return false;
	}

	{//检查是否存在于尖端交易池中
		lock_guard<mutex> guard(m_tips_pool_lock);
		for (Unit u : tipsPool_) {
			if (u.getHash() == hash) {
				return false;
			}
		}
	}

	//验证签名
	if (!KeyPair::verifySignature(js)) {
		return false;
	}

	//验证PoW
	if (!Consensus::verifyPoW(js)) {
		return false;
	}

	Unit newUnit(root);
	//将对应尖端放入数据库
	sha256_t tip1 = newUnit.getHheader().tipsHash[0];
	sha256_t tip2 = newUnit.getHheader().tipsHash[1];

	{
		std::lock_guard<std::mutex> guard(m_tips_pool_lock);
		for (auto i = tipsPool_.begin(); i < tipsPool_.end(); i++) {
			if ((*i).getHash() == tip1 || (*i).getHash() == tip2) {
				dag_db_.push((*i));
				////////////////重大BUG
				//放入数据库但没有擦出尖端
				tipsPool_.erase(i);
				if (tip1 == tip2)
					break;
				
			}
		}
	}

	//将新收到的单元放入尖端缓存中
	pushTip(newUnit);
	return true;
}

void Dag::pushTip(const Unit& newUnit) {
	std::lock_guard<std::mutex> guard(m_tips_pool_lock);
	tipsPool_.push_back(newUnit);	m_tips_pool_cond.notify_one();//产生新交易的时候需要选择尖端}//TODObool Dag::verifyTip(const Unit& tip) {
	if (tip.getHheader().tipsHash[0] == "0000000000000000000000000000000000000000000000000000000000000000") {
		return true;
	}

	Unit testUnit;
	if (dag_db_.getUnit(tip.getHash(), testUnit)) {
		return false;
	}
	return true;
}

//TODO 后期改为MCMC
bool Dag::getRandTip(uint32_t& index) {
	uint32_t size = tipsPool_.size();
	if (size < 1)
		return false; //没有尖端交易了
	std::vector<int> rand;
	for (uint32_t i = 0; i < size; i++)
		rand.push_back(i);
	random_shuffle(rand.begin(), rand.end());
	std::string  k;
	index = rand[0];
	return true;
}

bool Dag::selectTips(sha256_t (&tipsHash)[2]) { //多进程时资源竞争问题？
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
			//尖端合法
			tipsHash[tipCount] = tipsPool_[rand].getHash();
			{	
				std::lock_guard<std::mutex> guard(m_dag_db_lock);
				dag_db_.push(tipsPool_[rand]); 
			}
			++tipCount;
		}
		else {
		}
		//无论是否合法都应该擦出
		auto begin = tipsPool_.begin();
		tipsPool_.erase(begin + rand);
	}
	lock.unlock();
	if (tipsHash[0] == "")	//尖端缓冲区不存在合法交易
		return false;
	if (tipsHash[1] == "")	//只存在一个尖端
		tipsHash[1] = tipsHash[0];
	return true;
}

//获取最新区块hash和timastamp
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

void Dag::pushAllUnit(const Json::Value& root) {	if (root.empty()) {		//启动线程池，允许产生新Unit		thread_pool_.start();		return;	}	uint64_t newSize = root["database"].size();	uint64_t oldSize = getCount();	if (newSize > oldSize) {		dag_db_.pushAllUnit(root["database"]);	}	lock_guard<mutex> guard(m_tips_pool_lock);	newSize = root["tips"].size();	oldSize = tipsPool_.size();	if (newSize > oldSize) {		for (int i = 0; i < root["tips"].size(); ++i) {			Unit m_unit(root["tips"][i]);			tipsPool_.push_back(m_unit);		}	}	//启动线程池，允许产生新Unit	thread_pool_.start();}void Dag::pushInfo(const string& str){
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