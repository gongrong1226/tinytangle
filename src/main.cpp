#include <serial.h>
#include <tinytangle/keypair.h>
#include <tinytangle/transaction.h>
#include <tinytangle/unit.h>
#include <tinytangle/database.h>
#include <tinytangle/dag.h>
#include <threadpool.hpp>
#include <Network.h>
//ssl; crypto; boost_system;      //ld 

using namespace tangle;
using namespace embeded_data;
// global logger

//测试通过/*
static void testKeyDatabase(void) {/*
	database db;
	db.init();
	Dag dag(2018);
	dag.init();
	KeyPair A;
	dag.getNewKeyPair(A);

	KeyPair B;
	dag.getNewKeyPair(B);

	KeyPairDatabase keyDB;
	Transaction tx(A.address(), 233, B.address());
	keyDB.updAccount(tx);
	Transaction tx1(B.address(), 133, A.address());
	keyDB.updAccount(tx1);*/
	return;
}

//测试通过
static void testDag(void){
	const string UART_NAME = "/dev/ttyUSB0";

	//初始化本地数据库
	database db;
	db.init();

	threadpool::fixed_thread_pool m_thread_pool(2, 2);
	//初始化DAG，创建或同步节交易单元（同步是在NETWORK进行）
	Dag dag(2019);
	dag.init();

	serial uart("/dev/ttyUSB0", 115200, &dag);
	uart.info();
	uart.start_read();

	while (true) {/*
		s[0] = ZERO + count++;
		string str1(s);
		str1 = "test" + str1;
		Unit unit1;
		dag.pushInfo(str1);

		s[0] = ZERO + count++;
		string str2(s);
		str2 = "test" + str2;
		Unit unit2;
		dag.pushInfo(str2);

		s[0] = ZERO + count++;
		string str3(s);
		str3 = "test" + str3;
		Unit unit3;
		dag.pushInfo(str3);
		if (count > 9) {
			count %= 10;
			sleep(1);
		}*/
	}
	/*m_thread_pool.execute(bind([](Dag* dag, Transaction* payToB, private_key_t* keyAPrv) {
		Unit unit1;
		dag->creatUnit(unit1, *payToB, *keyAPrv);
		//交易单元转换json在网络中传输
		Json::Value root = unit1.to_json();

		///////////////网络另一端//////////////
		//收到Json报文，直接push到DAG
		//交由DAG进行验证， 网络只管收发
		Json::Value recv = root;
		if (dag->pushUnit(recv)) {
			//如过验证通过，继续广播该交易
			//broadcast(recv);
			//log::info("testDag") << "Broadcast unit";
			std::cout << "Broadcast unit" << std::endl;
		}
		cout << "test timer1:" << threadpool::fixed_thread_pool::get_current_ms() << endl;
	}, &dag, &payToB, &keyAPrv), "timer1", 2000);*/
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		cout << "no broadcast address...\n such like 192.168.255.255" << endl;
		return -1;
	}

	////初始化本地数据库
	database db;
	db.init();

	////初始化DAG，创建或同步节交易单元（同步是在NETWORK进行）
	Dag dag(2019);

	////网络侧初始化，同步Unit
	Network m_network(6000, string(argv[1]));

	//被请求同步时获取Unit数量
	m_network.set_get_units_count(std::bind(&Dag::getCount, &dag));
	//被请求同步时获取全部Unit
	m_network.set_get_all_units(std::bind(&Dag::getAllUnit, &dag, placeholders::_1));
	//请求同步后装入全部Unit
	m_network.set_push_all_unit(std::bind(&Dag::pushAllUnit, &dag, placeholders::_1));
	//设置网络侧接收到Unit过后的回调函数
	m_network.set_push_one_unit(std::bind(&Dag::pushUnit, &dag, placeholders::_1));

	//同步Unit
	m_network.sync_dag();

	//设置DAG广播回调
	dag.setBroadcastHandler(std::bind(&Network::broadcast_unit, &m_network, placeholders::_1));
	//初始化DAG
	dag.init();

	////监听，新Unit来源
	serial uart("/dev/ttyUSB0", 115200, &dag);
	uart.info();
	//按照"rf_rx_data"标头进行接收并打包
	uart.start_read(); 

	string ip_str;
	auto cmd_handle = [&](const string &cmd) {
		ip_str = m_network.get_ip();
		if (cmd == "quit") {
			exit(0);
		}
		else if (cmd == "pause") {
			log::info("cmd_handle") << "pause";
			dag.stopProdUnits();
		}
		else if (cmd == "once") {
			log::info("cmd_handle") << "once";
			dag.mineOnce(ip_str);
		}
		else if (cmd == "start") {
			log::info("cmd_handle") << "start";
			dag.startProdUnits(ip_str);
		}
		else if (cmd == "tips") {
			dag.showTips();
		}
		else {
			log::debug("cmd_handle") << "unknown cmd";
		}
	};
	m_network.set_push_cmd(cmd_handle);
	m_network.start_listen();


	while (true) {
		string cmd;
		cin >> cmd;
		if (cmd == "quit")
			break;
		cmd.clear();
	}

	//dag.~Dag();
	return 0;
}

