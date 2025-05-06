#pragma once
#include "Singleton.h"
#include <functional>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "mtconst.h"

class Log;

class Mt4MgrSvr :public Singleton<Mt4MgrSvr>
{
	friend class Singleton<Mt4MgrSvr>;
public:
	Mt4MgrSvr();
	~Mt4MgrSvr();
public:
	std::string m_svraddr;
	int m_user;
	std::string m_pwd;
	std::shared_ptr<Log> m_log;

public:

	bool Init();
	bool Connect();

	TradeRecord* GetTrades(int* total);
	TradeRecord* TradesUserHistory(const int login, const __time32_t from, const __time32_t to, int* total);
	TradeRecord* AdmTradesRequest(char* group, int* total);

	UserRecord* UsersRequest(int* total);

	int TradeTransaction(TradeTransInfo* info, char* msg);
	int UserRecordNew(UserRecord* user, char* msg);
	int UserRecordUpdate(UserRecord * user, char* msg);

	void MemFree(void* ptr);
};

