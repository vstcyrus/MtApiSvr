#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include "CSession.h"
#include <queue>
#include <map>
#include <functional>
#include "const.h"
#include "MT4ManagerAPI.h"
#include <filesystem>
#include <chrono>

class Log;
class Mt4MgrSvr;

typedef  function<void(shared_ptr<CSession>, const short& msg_id, const string& msg_data)> FunCallBack;
class LogicSyc:public Singleton<LogicSyc>
{
	friend class Singleton<LogicSyc>;
public:
	~LogicSyc();
	void PostMsgToQue(shared_ptr < LogicNode> msg);
private:
	LogicSyc();
	void DealMsg();
	void RegisterCallBacks();
	void Hert(shared_ptr<CSession>, const short& msg_id, const string& msg_data);
	void GetTrades(shared_ptr<CSession>, const short& msg_id, const string& msg_data);
	void GetHistoryTrades(shared_ptr<CSession>, const short& msg_id, const string& msg_data);

	void TradeTransaction(shared_ptr<CSession>, const short& msg_id, const string& msg_data);
	void UserRecordNew(shared_ptr<CSession>, const short& msg_id, const string& msg_data);
	void UserRecordUpdate(shared_ptr<CSession>, const short& msg_id, const string& msg_data);
	void GetUsersInfo(shared_ptr<CSession> session, const short& msg_id, const string& msg_data);
	void PumpTrades(shared_ptr<CSession>, const short& msg_id, const string& msg_data);

	std::string GetTradeStr(TradeRecord trade);
	std::string UserRecordToString(const UserRecord& user);

	std::thread _worker_thread;
	std::queue<shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _consume;
	bool _b_stop;
	std::map<short, FunCallBack> _fun_callbacks;
	static std::shared_ptr<Mt4MgrSvr> s_mt4MgrServer; 
	static std::shared_ptr<Log> s_Logger;
};
