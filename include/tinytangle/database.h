#pragma once
#include <mutex>
		//����timestamp��ȡ����UnitHash
		bool getLastUnit(int64_t& timestamp, sha256_t& hash);

		void getAllUnit(Json::Value& root) ;
		void pushAllUnit(const Json::Value& root);
			sqlite3pp::query qry(db_conn_, "SELECT * FROM key_pairs");
			auto i = qry.begin();
			auto priKeyBase64 = (*i).get<std::string>(1); //address private_key account
			sqlite3pp::query qry(db_conn_, "SELECT count(*) FROM key_pairs");