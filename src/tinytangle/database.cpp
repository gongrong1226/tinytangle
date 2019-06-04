#include <tinytangle/database.h>

namespace tangle {


// ----------------------- class databse -------------------
void database::print(){
    sqlite3pp::database db_conn{db_name_};
    log::debug("database")<<"====== Print begin =====";
    try{
        sqlite3pp::query qry(db_conn, "SELECT * FROM key_pairs");
        log::debug("database")<< "column count:"<< qry.column_count();

        for (int i = 0; i < qry.column_count(); ++i) {
	      log::debug("database") << "columm name:" << qry.column_name(i);
        }

        for (sqlite3pp::query::iterator i = qry.begin(); i != qry.end(); ++i) {
	        for (int j = 0; j < qry.column_count(); ++j) {
                log::debug("database")<< (*i).get<char const*>(j);
	        }
        }
    } catch (std::exception& ex) {
        db_conn.disconnect();
        log::error("database")<<"disconnect db with error:"<< ex.what();
    }
    db_conn.disconnect();
    log::debug("database")<<"====== Print end =====";
}

void database::init() {

	namespace bf = boost::filesystem;

	auto db_path = bf::current_path() / db_name_;

	if (bf::exists(db_path)) {
		log::info("database") << "Using " << db_name_;
		return;
	}

	log::info("database") << "Initializing " << db_name_;

	sqlite3pp::database db_conn{ db_name_ };

	try {
		sqlite3pp::command cmd(db_conn, "create table if not EXISTS Unit ( \
          hash char(64) not null primary KEY, \
          tiphash1 char(64) not null, \
          tiphash2 char(64) not null, \
          signature BLOB, \
		  time_stamp BIGINT ,	\
          nonce INTEGER , \
          difficulty INTEGER , \
          self_weight INTEGER , \
		  weight BIGINT NOT NULL DEFAULT 0, \
          payer BLOB, \
          message char(256), \
          payee BLOB, \
          value BIGINT, \
          version char(8) , \
		  hash_bk char(64) NOT NULL DEFAULT '0',\
		  weight_bk BIGINT NOT NULL DEFAULT 0, \
		  repeated_weight BIGINT DEFAULT 0, \
		  time datetime);");
		cmd.execute();

		sqlite3pp::command cmd2(db_conn, "create table if not EXISTS key_pairs( \
          address char(256) primary key, \
          private_key BLOB, \
          account bigint NOT NULL DEFAULT 0);");
		cmd2.execute();

		sqlite3pp::command cmd_indexes_hash(db_conn, "create index hash_index on Unit(hash);");
		cmd_indexes_hash.execute();

		sqlite3pp::command cmd_digui(db_conn, "PRAGMA recursive_triggers = true;");

		//添加函数
		/*
		std::function<int(std::string, int)> fun;
		fun = [&](std::string tiphash, int new_weight) {
			std::cout << "mfunc" << std::endl;
			sqlite3pp::transaction xct(db_conn);
			sqlite3pp::command update(db_conn, "UPDATE Unit SET weight = self_weight + ? WHERE hash = ? ;");
			update.binder() << new_weight << tiphash;
			update.execute();
			xct.commit();

			std::string str_qry("SELECT tiphash1,tiphash2,weight FROM Unit WHERE hash = ");
			std::string str = str_qry + tiphash;
			sqlite3pp::query qry(db_conn, str.c_str());//

			auto i = qry.begin();
			std::string tiphash1 = (*i).get<std::string>(0);
			std::string tiphash2 = (*i).get<std::string>(1);
			int weight = (*i).get<int>(2);
			fun(tiphash1, weight);
			if(tiphash1!=tiphash2)
				fun(tiphash2, weight);
			return 100;
		};
		sqlite3pp::ext::function func(db_conn);
		std::cout << func.create<int(std::string, int)>("mfunc", fun) << std::endl;

		auto handle_update = [&](int opcode, char const* dbname, char const* tablename, long long int rowid) {
			std::string table_str(tablename);
			std::cout << "handle_update" << std::endl;
			if (opcode == SQLITE_INSERT && table_str == "Unit") {
				std::stringstream sstr;
				sstr << (rowid - 1);
				std::string str = sstr.str();
				std::string str_qry("SELECT tiphash1,tiphash2,weight FROM Unit LIMIT 1 OFFSET ");
				str_qry = str_qry + str + ";";

				sqlite3pp::query qry(db_conn, str.c_str());//
				auto i = qry.begin();
				std::string tiphash1 = (*i).get<std::string>(0);
				std::string tiphash2 = (*i).get<std::string>(1);
				int weight = (*i).get<int>(2);

				//tiphash1前向weight更新
				sstr.clear();
				sstr << "SELECT mfunc(" << tiphash1 << "," << weight << ")";
				str_qry = sstr.str();
				sqlite3pp::query qry_tip1(db_conn, str.c_str());//

				if (tiphash1 == tiphash2)
					return;
				sstr.clear();
				sstr << "SELECT mfunc(" << tiphash2 << "," << weight << ")";
				str_qry = sstr.str();
				sqlite3pp::query qry_tip2(db_conn, str.c_str());//

			}
		};
		db_conn.set_update_handler(handle_update);
		auto handle_authorize = [&](int evcode, char const* p1, char const* p2, char const* dbname, char const* tvname) {
			std::cout << "handle_authorize(" << evcode << ")" << std::endl;
			return 0;
		};
		db_conn.set_authorize_handler(handle_authorize); */
		/*
		db_conn.execute("INSERT INTO Unit (hash) VALUES ('AAAA')");
		{
			sqlite3pp::transaction xct(db_conn);
			sqlite3pp::command cmd(db_conn, "INSERT INTO Unit (hash) VALUES ('AAAA')");
			std::cout << cmd.execute() << std::endl;
			xct.commit();
		}
		*/
		//自重更新过后前向更新
		/*sqlite3pp::command cmd3(db_conn, "CREATE TRIGGER if not EXISTS weight_update_double_tips \
			AFTER UPDATE OF weight ON Unit \
			FOR EACH ROW \
			WHEN new.tiphash1 != new.tiphash2 \
			BEGIN \
				UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash1; \
				UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash2; \
				UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash1; \
				UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash2; \
			END; ");
		cmd3.execute();
		sqlite3pp::command cmd3_tips(db_conn, "CREATE TRIGGER if not EXISTS weight_update_single_tip \
			AFTER UPDATE OF weight ON Unit \
			FOR EACH ROW \
			WHEN new.tiphash1 = new.tiphash2 \
			BEGIN \
				UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash1; \
				UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash1; \
			END; ");
		cmd3_tips.execute();
		sqlite3pp::command cmd3_bk(db_conn, "CREATE TRIGGER if not EXISTS weight_bk_update_double_tips \
			AFTER UPDATE OF weight_bk ON Unit \
			FOR EACH ROW \
			WHEN new.tiphash1 != new.tiphash2 \
			BEGIN \
				UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash1; \
				UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash2; \
				UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash1; \
				UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash2; \
			END; ");
		cmd3_bk.execute();
		sqlite3pp::command cmd3_tips_bk(db_conn, "CREATE TRIGGER if not EXISTS weight_bk_update_single_tip \
			AFTER UPDATE OF weight_bk ON Unit \
			FOR EACH ROW \
			WHEN new.tiphash1 = new.tiphash2 \
			BEGIN \
				UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash1; \
				UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash1; \
			END; ");
		cmd3_tips_bk.execute();*/

		//插入新的单元过后自重前向更新
		//sqlite3pp::command cmd4(db_conn, "CREATE TRIGGER if not EXISTS weight_update_after_insert_double_tips \
		//	AFTER INSERT ON Unit \
		//	WHEN new.tiphash1 != new.tiphash2\
		//	BEGIN \
		//		SELECT  mfunc(new.tiphash1,new.weight); \
		//		SELECT  mfunc(new.tiphash2,new.weight); \
		//	END; ");
		//cmd4.execute();
		//sqlite3pp::command cmd4_tips(db_conn, "CREATE TRIGGER if not EXISTS weight_update_after_insert_single_tip\
		//	AFTER INSERT ON Unit \
		//	WHEN new.tiphash1 = new.tiphash2\
		//	BEGIN \
		//		SELECT  mfunc('1', 1); \
		//	END; ");
		//cmd4_tips.execute();

	}
	catch (std::exception& ex) {
		db_conn.disconnect();
		log::error("database") << "disconnect db with error:" << ex.what();
	}

	db_conn.disconnect();

	//log::info("database") << "Creating genesis block";
	log::info("database") << "Finish initializing";
}
DagDatabase::DagDatabase() {	/*std::function<int(std::string, int)> fun;
	fun = [&](std::string tiphash, int new_weight) {
		std::cout << "mfunc" << std::endl;
		sqlite3pp::transaction xct(db_conn_);
		sqlite3pp::command update(db_conn_, "UPDATE Unit SET weight = self_weight + ? WHERE hash = ? ;");
		update.binder() << new_weight << tiphash;
		update.execute();
		xct.commit();

		std::string str_qry("SELECT tiphash1,tiphash2,weight FROM Unit WHERE hash = ");
		std::string str = str_qry + tiphash;
		sqlite3pp::query qry(db_conn_, str.c_str());//

		auto i = qry.begin();
		std::string tiphash1 = (*i).get<std::string>(0);
		std::string tiphash2 = (*i).get<std::string>(1);
		int weight = (*i).get<int>(2);
		fun(tiphash1, weight);
		if (tiphash1 != tiphash2)
			fun(tiphash2, weight);
		return 100;
	};
	sqlite3pp::ext::function func(db_conn_);
	std::cout << func.create<int(std::string, int)>("mfunc", fun) << std::endl;

	auto handle_update = [&](int opcode, char const* dbname, char const* tablename, long long int rowid) {
		std::string table_str(tablename);
		std::cout << "handle_update" << std::endl;
		if (opcode == SQLITE_INSERT && table_str == "Unit") {
			std::stringstream sstr;
			sstr << (rowid - 1);
			std::string str = sstr.str();
			std::cout << getCount() << std::endl;
			std::string str_qry("SELECT tiphash1,tiphash2,weight FROM Unit LIMIT 1 OFFSET ");
			str_qry = str_qry + str + ";";
			str_qry = "SELECT tiphash1,tiphash2,weight FROM Unit limit 0,1";
			auto pch = str.c_str();
			std::cout << str_qry << std::endl;

			sqlite3pp::query qry(db_conn_, pch);//
			auto i = qry.begin();
			std::string tiphash1 = (*i).get<std::string>(0);
			std::string tiphash2 = (*i).get<std::string>(1);
			int weight = (*i).get<int>(2);

			//tiphash1前向weight更新
			sstr.clear();
			sstr << "SELECT mfunc('" << tiphash1 << "'," << weight << ")";
			str_qry = sstr.str();
			std::cout << str_qry << std::endl;
			pch = str.c_str();
			sqlite3pp::query qry_tip1(db_conn_, pch);//

			if (tiphash1 == tiphash2)
				return;
			sstr.clear();
			sstr << "SELECT mfunc('" << tiphash2 << "'," << weight << ")";
			str_qry = sstr.str();
			sqlite3pp::query qry_tip2(db_conn_, str.c_str());//

		}
	};
	db_conn_.set_update_handler(handle_update);
	auto handle_authorize = [&](int evcode, char const* p1, char const* p2, char const* dbname, char const* tvname) {
		std::cout << "handle_authorize(" << evcode << ")" << std::endl;
		return 0;
	};
	db_conn_.set_authorize_handler(handle_authorize);*/


	sqlite3pp::command cmd_recursive_triggers(db_conn_, "PRAGMA recursive_triggers = true;");


	//std::function<int(std::string )> update_weight_handle;
	//update_weight_handle = [db_conn_&](std::string hash) {
	//	std::cout << "update_weight" << std::endl;
	//	sqlite3pp::transaction xct(db_conn_);
	//	sqlite3pp::command update(db_conn_, "\
	//		WITH RECURSIVE \
	//		  temp_unit AS( \
	//		  	SELECT * FROM Unit WHERE hash = ? \
	//			UNION \
	//			SELECT Unit.*FROM temp_unit, Unit WHERE Unit.hash = temp_unit.tiphash1 OR Unit.hash = temp_unit.tiphash2 \
	//		  ) \
	//		UPDATE  Unit set weight = weight + (SELECT self_weight FROM Unit WHERE hash = ? ) \
	//		  WHERE hash = (SELECT hash FROM temp_unit where temp_unit.hash = Unit.hash)");
	//	update.binder() << hash << hash;
	//	update.execute();
	//	xct.commit();
	//	return 0;
	//};
	//auto fun = std::bind(update_weight_handle, this, std::placeholders::_1);
	//sqlite3pp::ext::function func(db_conn_);
	//std::cout << func.create<int(std::string)>("update_weight", update_weight_handle) << std::endl;
	////添加新Unit后触发调用update_weight函数进行更新
	//sqlite3pp::command cmd4(db_conn_, "CREATE TRIGGER if not EXISTS weight_update_after_insert_double_tips \
	//		AFTER INSERT ON Unit \
	//		BEGIN \
	//		  SELECT update_weight(new.hash); \
	//		END; ");
	//cmd4.execute();

	//触发器更新，dfs 受深度 SQLITE_MAX_TRIGGER_DEPTH  限制 
	sqlite3pp::command cmd3(db_conn_, "CREATE TRIGGER if not EXISTS weight_update_double_tips \
		AFTER UPDATE OF hash_bk ON Unit \
		for each row \
		WHEN old.hash_bk != new.hash_bk AND new.tiphash1 != new.tiphash2 \
		BEGIN \
			UPDATE Unit SET weight = weight + new.weight_bk WHERE hash = new.tiphash1; \
			UPDATE Unit SET weight_bk = new.weight_bk WHERE hash = new.tiphash1; \
			UPDATE Unit SET hash_bk = new.hash_bk WHERE hash = new.tiphash1; \
			UPDATE Unit SET weight = weight + new.weight_bk WHERE hash = new.tiphash2; \
			UPDATE Unit SET weight_bk = new.weight_bk WHERE hash = new.tiphash2; \
			UPDATE Unit SET hash_bk = new.hash_bk WHERE hash = new.tiphash2; \
		END; ");
	cmd3.execute();
	sqlite3pp::command cmd3_tips(db_conn_, "CREATE TRIGGER if not EXISTS weight_update_single_tip \
		AFTER UPDATE OF hash_bk ON Unit \
		for each row \
		WHEN old.hash_bk != new.hash_bk AND new.tiphash1 = new.tiphash2 \
		BEGIN \
			UPDATE Unit SET weight = weight + new.weight_bk WHERE hash = new.tiphash1; \
			UPDATE Unit SET weight_bk = new.weight_bk WHERE hash = new.tiphash1; \
			UPDATE Unit SET hash_bk = new.hash_bk WHERE hash = new.tiphash1; \
		END; ");
	cmd3_tips.execute(); 
	sqlite3pp::command cmd3_delete_repeat(db_conn_, "CREATE TRIGGER if not EXISTS weight_update_delete_repeat \
		AFTER UPDATE OF hash_bk ON Unit \
		for each row \
		WHEN old.hash_bk = new.hash_bk \
		BEGIN \
			UPDATE Unit SET weight = weight - weight_bk WHERE hash = new.hash; \
		END; ");
	cmd3_delete_repeat.execute();//UPDATE Unit SET repeated_weight = repeated_weight + weight_bk WHERE hash = new.hash; 
	/*
	sqlite3pp::command cmd3_bk(db_conn_, "CREATE TRIGGER if not EXISTS weight_bk_update_double_tips \
		AFTER UPDATE OF weight_bk ON Unit \
		FOR EACH ROW \
		WHEN new.tiphash1 != new.tiphash2 \
		BEGIN \
			UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash1; \
			UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash2; \
			UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash1; \
			UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash2; \
		END; ");
	cmd3_bk.execute();
	sqlite3pp::command cmd3_tips_bk(db_conn_, "CREATE TRIGGER if not EXISTS weight_bk_update_single_tip \
		AFTER UPDATE OF weight_bk ON Unit \
		FOR EACH ROW \
		WHEN new.tiphash1 = new.tiphash2 \
		BEGIN \
			UPDATE Unit SET weight = self_weight + new.weight WHERE hash = new.tiphash1; \
			UPDATE Unit SET weight_bk = weight WHERE hash = new.tiphash1; \
		END; ");
	cmd3_tips_bk.execute();*/

	//插入新的单元过后更新weight
	sqlite3pp::command cmd4_tips(db_conn_, "CREATE TRIGGER if not EXISTS weight_update_after_insert_single_tip\
			AFTER INSERT ON Unit \
			BEGIN \
				UPDATE Unit SET time = datetime('now','localtime') WHERE hash = new.hash; \
				UPDATE Unit SET weight = new.weight + new.self_weight WHERE hash = new.hash; \
				UPDATE Unit SET weight_bk = new.self_weight WHERE hash = new.hash; \
				UPDATE Unit SET hash_bk = new.hash WHERE hash = new.hash; \
			END; "); //如果调换weight_bk 和hash_bk的UPDATE顺序，后续所有unit都无法正常更新，说明触发器的递归和C/C++的递归原则一样
	cmd4_tips.execute();}//向前更新1000个 大约void DagDatabase::updateWeright(const std::string &hash) {
	//sqlite3pp::transaction xct(db_conn_);
	sqlite3pp::command update(db_conn_, "\
			WITH RECURSIVE \
			  temp_unit AS( \
			  	SELECT * FROM Unit WHERE hash = ? \
				UNION \
				SELECT Unit.*FROM temp_unit, Unit WHERE Unit.hash = temp_unit.tiphash1 OR Unit.hash = temp_unit.tiphash2 \
				LIMIT 1000\
			  ) \
			UPDATE  Unit set repeated_weight = repeated_weight + (SELECT self_weight FROM Unit WHERE hash = ? ) \
			  WHERE hash = (SELECT hash FROM temp_unit where temp_unit.hash = Unit.hash);");
	update.binder() << hash << hash;
	update.execute();
	//xct.commit();}//向后查看,检验weight是否正确，不做限制，但1000行数据大约75msvoid DagDatabase::check_weight(const std::string &hash) {
	sqlite3pp::transaction xct(db_conn_);
	sqlite3pp::command update(db_conn_, "\
			WITH RECURSIVE \
			  temp_unit AS( \
			  	SELECT * FROM Unit WHERE hash = ? \
				UNION \
				SELECT Unit.* FROM temp_unit, Unit WHERE Unit.tiphash1 = temp_unit.hash OR Unit.tiphash2 = temp_unit.hash  \
			  ) \
			select self_weight,sum(self_weight) from temp_unit group by self_weight;");
	update.binder() << hash;
	update.execute();
	xct.commit();}bool DagDatabase::push(const Unit& newUnit) {//Json::Value更合适
	std::lock_guard<std::mutex> lock(update_weight_mtx_);
	sqlite3pp::transaction xct(db_conn_);
	sqlite3pp::command cmd(db_conn_, "INSERT INTO Unit (\
						hash, tiphash1, tiphash2, signature,time_stamp, nonce, difficulty, self_weight, version,\
						payer, message, payee,value\
						) VALUES (?, ?, ?, ?, ?, ?, ? ,? ,? ,? ,? ,?,?)");
	//md5不可恢复，如果要可视化应该用另外的编码
	//payer_address payer = newUnit.getTransaction().getPayer(); //获取地址，因为DERencode的原因，是乱码
	//payee_address payee = newUnit.getTransaction().getPayee();	
	//addr2String(payer, payer); //将乱码转换可视化字符， code->pubkey->md5
	//addr2String(payee, payee); 	
	cmd.binder() << newUnit.header_.hash									//height
		<< newUnit.header_.tipsHash[0]											//tiphash1
		<< newUnit.header_.tipsHash[1] 										//tiphash2
		<< newUnit.header_.signature
		<< static_cast<long long>(newUnit.header_.timestamp)										//time_stamp
		<< static_cast<int>(newUnit.header_.nonce)											//nonce
		<< static_cast<int>(newUnit.header_.difficulty)										//difficulty
		<< static_cast<int>(newUnit.header_.selfWeight)										//self weight*/
		<< newUnit.header_.version											//version
		<< newUnit.getTransaction().getPayer()														//payer;
		<< newUnit.getTransaction().getMessage()												//payee
		<< newUnit.getTransaction().getPayee()														//payee
		<< static_cast<long long>(newUnit.getTransaction().getValue());

	log::debug("DagDatabase") << "ADD UNIT " << newUnit.header_.hash;
	cmd.execute();
	updateWeright(newUnit.header_.hash);
	xct.commit();
}

void DagDatabase::getAllUnit(Json::Value& root)  {
	int n = 0;
	sqlite3pp::query qry(db_conn_, "SELECT * FROM Unit");//
	for (auto i = qry.begin(); i != qry.end(); ++i) { //和建表顺序要一一对应，数据类型也要匹配
		root[n]["header"]["hash"] = (*i).get<std::string>(0);
		root[n]["header"]["tipsHash"][1] = (*i).get<std::string>(1);
		root[n]["header"]["tipsHash"][2] = (*i).get<std::string>(2);
		//root[n]["header"]["signature"] = (*i).get<std::string>(3);
		root[n]["header"]["time_stamp"] = static_cast<int64_t>((*i).get<long long int>(4));
		root[n]["header"]["nonce"] = static_cast<uint32_t>((*i).get<int>(5));
		root[n]["header"]["difficulty"] = static_cast<uint32_t>((*i).get<int>(6));
		root[n]["header"]["self_weight"] = static_cast<uint32_t>((*i).get<int>(7));
		root[n]["header"]["version"] = (*i).get<std::string>(13);//5/20更改 
		//root[n]["tx"]["tx_item"]["payer"] = (*i).get<std::string>(9);
		root[n]["tx"]["message"] = (*i).get<std::string>(10);
		//root[n]["tx"]["tx_item"]["payee"] = (*i).get<std::string>(11);
		root[n]["tx"]["tx_item"]["value"] = static_cast<int64_t>((*i).get<long long int>(12));

		//colum3 signature
		int signature_length = (*i).column_bytes(3);
		void const  *signature_blob = NULL;
		signature_blob = (*i).get(3, signature_blob);
		std::string signature_str;
		signature_str.insert(0, (const char*)signature_blob, signature_length);
		root[n]["header"]["signature"] = signature_str;

		//colum9 payer
		int payer_length = (*i).column_bytes(9);
		void const  *payer_blob = NULL;
		payer_blob = (*i).get(9, payer_blob);
		std::string payer_str;
		payer_str.insert(0, (const char*)payer_blob, payer_length);
		root[n]["tx"]["tx_item"]["payer"] = payer_str;

		//colum11 payee
		int payee_length = (*i).column_bytes(11);
		void const  *payee_blob = NULL;
		payee_blob = (*i).get(11, payee_blob);
		std::string payee_str;
		payee_str.insert(0, (const char*)payee_blob, payee_length);
		root[n]["tx"]["tx_item"]["payee"] = payee_str;

		n++;
		//hash = (*i).get<const char*>(1);
	}
}

void DagDatabase::pushAllUnit(const Json::Value& root) {
	sqlite3pp::transaction xct(db_conn_);
	if(getCount()!=0){
		sqlite3pp::command cmd(db_conn_, "DELETE * FROM Unit");
		cmd.execute();
	}
	for (int i = 0; i < root.size(); ++i) {
		sqlite3pp::command cmd(db_conn_, "INSERT INTO Unit (\
						hash, tiphash1, tiphash2, signature,time_stamp, nonce, difficulty, self_weight, version,\
						payer, message, payee,value\
						) VALUES (?, ?, ?, ?, ?, ?, ? ,? ,? ,? ,? ,?,?)");
		cmd.binder() << root[i]["header"]["hash"].asString()								//hash
			<< root[i]["header"]["tipsHash"][1].asString()									//tiphash1
			<< root[i]["header"]["tipsHash"][2].asString()									//tiphash2
			<< root[i]["header"]["signature"].asString()
			<< static_cast<long long>(root[i]["header"]["time_stamp"].asInt64())					//time_stamp
			<< static_cast<int>(root[i]["header"]["nonce"].asInt())								//nonce
			<< static_cast<int>(root[i]["header"]["difficulty"].asInt())							//difficulty
			<< static_cast<int>(root[i]["header"]["self_weight"].asInt())						//self weight*/
			<< root[i]["header"]["version"].asString()										//version
			<< root[i]["tx"]["tx_item"]["payer"].asString()									//payer;
			<< root[i]["tx"]["message"].asString()											//message
			<< root[i]["tx"]["tx_item"]["payee"].asString()									//payee
			<< static_cast<long long>(root[i]["tx"]["tx_item"]["value"].asInt64());
		cmd.execute();
		updateWeright(root[i]["header"]["hash"].asString());
	}
	xct.commit();
}

void DagDatabase::createGenesisUnit(void) {
	Unit genesisUnit;
	const address_t genesisAddr = "0000000000000000000000000000000000000000000000000000000000000000";
	Transaction tx(genesisAddr, "Creat Genesis Unit");
	
	genesisUnit.header_.tipsHash[0] = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.tipsHash[1] = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.signature = "0000000000000000000000000000000000000000000000000000000000000000";
	genesisUnit.header_.timestamp = get_now_timestamp();
	genesisUnit.header_.nonce = 0;
	genesisUnit.header_.difficulty = 1;
	genesisUnit.header_.selfWeight = 1;
	genesisUnit.header_.hash = getHash256(genesisUnit.to_json());
	genesisUnit.setup(tx);

	push(genesisUnit);
}

bool DagDatabase::getLastUnit(int64_t& timestamp, sha256_t& hash) {
	//sqlite3pp::query qry(db_conn_, "SELECT hash, time_stamp FROM Unit where time_stamp = (SELECT count(*) - 1 FROM Unit)");
 	sqlite3pp::query qry(db_conn_, "SELECT time_stamp,hash FROM Unit order by time_stamp desc limit 0,1");//0为偏移，1为条数
	for (auto i = qry.begin(); i != qry.end(); ++i) {
		timestamp = (*i).get<long long int>(0);
		hash = (*i).get<const char*>(1);
	}
	log::info("database") << "last_block_hash:" << hash;	return true;}uint64_t DagDatabase::getCount() {
	sqlite3pp::query qry(db_conn_, "SELECT count(*) FROM Unit");	auto i = qry.begin();	uint64_t count = (*i).get<long long int>(0);	return count;}//TODObool DagDatabase::getUnit(const sha256_t& hash, Unit& dest) {

	std::string cmdstr = "SELECT count(*) FROM Unit where hash = '" + hash + "'";
	sqlite3pp::query qry(db_conn_, cmdstr.c_str());	auto it = qry.begin();	uint64_t count = (*it).get<long long int>(0);
	if (count < 1) {
		return false;
	}
	else {
		return true;
	}
	//try
	//{
	//	std::string cmdstr = "SELECT count(*) FROM Unit where hash = '" + hash + "'";
	//	sqlite3pp::query qry(db_conn_, cmdstr.c_str());	//	auto i = qry.begin();
	//}
	//catch (const std::exception& e) {
	//	log::error(__FUNCTION__) << e.what();
	//	return false;
	//}
}
KeyPair KeyPairDatabase::getNewKeyPair() {
	KeyPair key; //随机私钥
	auto&& keypair = key.encode_pair(); //encode 64
	std::string prvKey = keypair.first;
	std::string pubKey = keypair.second;

	log::info(__FUNCTION__) << "get new key address: " << pubKey;
	sqlite3pp::transaction xct(db_conn_);
	sqlite3pp::command cmd(db_conn_, "INSERT INTO key_pairs (address, private_key) VALUES (?, ?)");
	//cmd.binder() << key.address() << keypair.first;
	cmd.binder() << pubKey << prvKey;
	cmd.execute();
	xct.commit();

	return key;
}
//正确情况应该是在DagDatabase更新的时候就触发交易更新bool KeyPairDatabase::updAccount(const Transaction& tx) {
	//DEREncodePublicKey -> pubkey
	payer_address payerStr = tx.getPayer();
	payee_address payeeStr = tx.getPayee();
	value v = tx.getValue();
	public_key_t payerKey, payeeKey;
	KeyPair::AddressToKey(payerStr, payerKey);
	KeyPair::AddressToKey(payeeStr, payeeKey);

	//pubkey-> Base64Encoder
	std::string encodePayer, encodePayee;
	CryptoPP::Base64Encoder payerSlink(new CryptoPP::StringSink(encodePayer), false);//false for no '\n'
	CryptoPP::Base64Encoder payeeSlink(new CryptoPP::StringSink(encodePayee), false);//false for no '\n'
	payerKey.DEREncode(payerSlink);
	payeeKey.DEREncode(payeeSlink);
	payerSlink.MessageEnd();//base64 编码补足
	payeeSlink.MessageEnd();//

	//检验账户和未确认的交易
	expenses(encodePayer, v);
	income(encodePayee, v);
	return true;
}
bool KeyPairDatabase::getBalance(const address_t& encodePubKey, value& balance) {
	try
	{
		std::string cmdstr = "SELECT account FROM key_pairs where address = '" + encodePubKey + "'";
		sqlite3pp::query qry(db_conn_, cmdstr.c_str());		auto i = qry.begin();		balance = (*i).get<long long int>(0);
	} catch (const std::exception& e) {
		log::error(__FUNCTION__) << e.what();
		return false;
	}	return true;}static bool isGenesis(std::string str) {	int index = 0;	while (str[index] == '0') {		index++;	}	if (index > 32)		return true;}//测试通过bool KeyPairDatabase::expenses(const std::string encodePubKey, const value v) {
	if (isGenesis(encodePubKey))
		return true;
	try
	{
		value balance;
		if (!getBalance(encodePubKey, balance))
			return false;
		balance -= v;

		//www.runoob.com/sqlite/sqlite-update.html
		//cmdstr ="UPDATE key_pairs (account) VALUES (?) where address = '" + encodePubKey + "'";
		std::string cmdstr = "";
		cmdstr = "UPDATE key_pairs SET account = "+ std::to_string(balance) + " where address = '" + encodePubKey + "'";
		sqlite3pp::command cmd(db_conn_, cmdstr.c_str());
		cmd.execute();
	} catch (const std::exception& ex){
		db_conn_.disconnect();
		log::error(__FUNCTION__) << "disconnect db with error: " << ex.what();
		return false;
	}	return true;

}//测试通过bool KeyPairDatabase::income(const std::string encodePubKey, const value v) {
	if (isGenesis(encodePubKey))
		return true;
	try
	{
		value balance;
		if (!getBalance(encodePubKey, balance))
			return false;
		balance += v;

		//www.runoob.com/sqlite/sqlite-update.html
		//cmdstr = "UPDATE key_pairs (account VALUES (?) where address = '" + encodePubKey + "'";
		std::string cmdstr = "";
		cmdstr = "UPDATE key_pairs SET account = " + std::to_string(balance) + " where address = '" + encodePubKey + "'";
		sqlite3pp::command cmd(db_conn_, cmdstr.c_str());
		cmd.execute();

	}
	catch (const std::exception& ex) {
		db_conn_.disconnect();
		log::error(__FUNCTION__) << "disconnect db with error: " << ex.what();
		return false;
	}	return true;}
}