#pragma once
#include <string>
#include <tinytangle/logging.hpp>
#include <json/json.h>
//#include <tinytangle/unit.h>

namespace tangle {


class Consensus {
public:
	static void PoW(Unit& unit);
	static bool verifyPoW(Json::Value root);
};

}
