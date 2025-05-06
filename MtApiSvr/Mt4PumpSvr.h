#pragma once
//#include <winsock2.h>
//#include <windows.h>
//#include <stdlib.h>
#include "Singleton.h"
#include "mtconst.h"
#include <iostream>
#include "CSession.h"

class Log;

enum {
	//OP_NOTSET = 0,
	OP_OPEN = 0,
	OP_CLOSE = 1,
	OP_UPDATE = 2,
	OP_NONE,
};
class Mt4PumpSvr:public Singleton<Mt4PumpSvr>
{
	friend Singleton<Mt4PumpSvr>;
public:
	Mt4PumpSvr();
	~Mt4PumpSvr();
public:
	std::string m_svraddr;
	int m_user;
	std::string m_pwd;
	std::shared_ptr<Log> m_log;
	

public:
	
	bool Init();
	bool Connect();
	void CheckConnect();
	void FreePump();
	static void __stdcall OnPumpingExFunc(int code, int type, void* data, void* param);

	void OnPumpStart(int code, int type, void* data);
	void OnPumpStop();

	void OnUserUpdate(int code, int type, void* data);
	void OnlineRecord(int code, int type, void* data);
	void OnTradeUpdate(int code, int type, void* data);

	void PumpTrades(shared_ptr<CSession>, const short& msg_id, const string& msg_data);


	std::string GetTradeStr(TradeRecord trade);

private:
	
};

