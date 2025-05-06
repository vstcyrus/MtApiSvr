#include "LogicSyc.h"
#include "Mt4MgrSvr.h"
#include "Mt4PumpSvr.h"
#include <wchar.h>
#include "errcode.h"
#include "util.h"
#include "Log.h"

using namespace std;

std::shared_ptr<Mt4MgrSvr> LogicSyc::s_mt4MgrServer = nullptr;
std::shared_ptr<Log>  LogicSyc::s_Logger = nullptr;

LogicSyc::LogicSyc() :_b_stop(false) {

	RegisterCallBacks();
	_worker_thread = std::thread(&LogicSyc::DealMsg, this);

}

LogicSyc::~LogicSyc() {
	_b_stop = true;
	_consume.notify_one();
	_worker_thread.join();
}

void LogicSyc::PostMsgToQue(shared_ptr < LogicNode> msg) {
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);
	if (_msg_que.size() == 1) {
		unique_lk.unlock();
		_consume.notify_one();
	}
}

void LogicSyc::DealMsg() {

	s_mt4MgrServer = std::make_shared<Mt4MgrSvr>();
	s_mt4MgrServer->Init();

	s_Logger = std::make_shared<Log>();
	s_Logger->LogicSycInit();

	for (;;) {
		std::unique_lock<std::mutex> unique_lk(_mutex);
		while (_msg_que.empty() && !_b_stop) {
			_consume.wait(unique_lk);
		}

		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
				auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _fun_callbacks.end()) {
					_msg_que.pop();
					continue;
				}
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		auto msg_node = _msg_que.front();
		cout << "recv_msg id  is " << msg_node->_recvnode->_msg_id << endl;
		auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _fun_callbacks.end()) {
			_msg_que.pop();
			std::cout << "msg id [" << msg_node->_recvnode->_msg_id << "] handler not found" << std::endl;
			continue;
		}
		call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSyc::RegisterCallBacks() {
	
	_fun_callbacks[MSG_HEART] = std::bind(&LogicSyc::Hert, this,
			placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[MSG_GET_TRADES] = std::bind(&LogicSyc::GetTrades, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[MSG_GET_TRADESHISTORY] = std::bind(&LogicSyc::GetHistoryTrades, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[MSG_BALANCE] = std::bind(&LogicSyc::TradeTransaction, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[MSG_NEWUSER] = std::bind(&LogicSyc::UserRecordNew, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[MSG_UPDATEUSER] = std::bind(&LogicSyc::UserRecordUpdate, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[MSG_GET_USERS_INFO] = std::bind(&LogicSyc::GetUsersInfo, this,
		placeholders::_1, placeholders::_2, placeholders::_3);	

	_fun_callbacks[MSG_Pump_TRADE] = std::bind(&LogicSyc::PumpTrades, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
}

void LogicSyc::Hert(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	//LOG_INFO(std::string("recive 1000 header pkg ."));
	std::cout << "revc header" << endl;
	session->Send("revc header", msg_id);
}

void LogicSyc::GetTrades(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	//Json::Reader reader;
	//Json::Value root;
	//reader.parse(msg_data, root);
	std::cout << "recevie msg id  is " << msg_data << " msg data is " << endl;
	//root["data"] = "server has received msg, msg data is " + root["data"].asString();
	//std::string return_str = root.toStyledString();
	int total = 0;
	TradeRecord * trades = s_mt4MgrServer->GetTrades(&total);

	int result = std::ceil(static_cast<double>(total) / 10);
	for (int i = 0;i < result; i++) {
		std::ostringstream oss;
		oss << Err_Success << "|Call succeeded" << Split_Record;
		int min = i * 10;
		int max = (i + 1) * 10;
		for (int j = min; j < max; j++) {
			if (j >= total) break;
			oss << GetTradeStr(trades[j]);
			// add log
			if (j < max -1) {
				oss << Split_Record;
			}
		}
		//oss << trades[i].login <<Split_Field << trades[i].symbol <<Split_Field << trades[i].order;
		std::string formattedString = oss.str();
		session->Send(formattedString, msg_id);
	}
}

std::string LogicSyc::GetTradeStr(TradeRecord trade) {
	std::ostringstream tradeoss;
	//std::string comment = util::GbkToUtf8(trade.comment);
	//std::cout << "comment: " << comment << endl;
	std::string splitStr =Split_Field;
	tradeoss << trade.order << Split_Field << trade.login << Split_Field << trade.symbol << Split_Field << trade.digits << Split_Field
		<< trade.cmd << Split_Field << trade.volume << Split_Field << trade.open_time << Split_Field << trade.open_price <<Split_Field
		<< trade.sl <<Split_Field << trade.tp <<Split_Field << trade.close_time <<Split_Field << trade.close_price <<Split_Field
		<< trade.reason <<Split_Field << trade.conv_rates[0] <<Split_Field << trade.conv_rates[1] <<Split_Field << trade.comment <<Split_Field
		<< trade.commission <<Split_Field << trade.commission_agent <<Split_Field << trade.storage <<Split_Field << trade.profit <<Split_Field
		<< trade.taxes <<Split_Field << trade.margin_rate <<Split_Field << trade.magic <<Split_Field << trade.gw_volume <<Split_Field
		<< trade.gw_open_price <<Split_Field << trade.gw_close_price <<Split_Field << trade.timestamp ;
	return tradeoss.str();
}

void LogicSyc::GetHistoryTrades(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {

	_LOG_INFO(LogicSyc::s_Logger, std::string("recevie msg id  is") + msg_data);

	std::ostringstream errmsg;
	std::vector<std::string> tokens;
	boost::split(tokens, msg_data, boost::is_any_of("|"));
	if (tokens.size() != 3) {
		errmsg << Err_ParamsNum << "|Parameters error";
		_LOG_INFO(LogicSyc::s_Logger, "Parameters error");
		session->Send(errmsg.str(), msg_id);
		return;
	} 
	int login = 0;
	long from, to;
	try {

		login = std::stoi(tokens[0]);
		from = std::stol(tokens[1]);
		to = std::stol(tokens[2]);
	}
	catch (const std::invalid_argument& e) {
		errmsg << Err_TypeConversion << "|Invalid argument:" << e.what();
		_LOG_INFO(LogicSyc::s_Logger, errmsg.str());
		session->Send(errmsg.str(), msg_id);
		return;
	}
	catch (const std::out_of_range& e) {
		errmsg << Err_TypeConversion << "|out of range:" << e.what();
		_LOG_INFO(LogicSyc::s_Logger, errmsg.str());
		session->Send(errmsg.str(), msg_id);
		return;
	}
	
	int total = 0;
	try {

		TradeRecord* trades = s_mt4MgrServer->TradesUserHistory(login, from, to, &total);
		_LOG_INFO(LogicSyc::s_Logger, std::string("Order total : ") + std::to_string(total));
		int result = std::ceil(static_cast<double>(total) / 10);
		for (int i = 0;i < result; i++) {
			std::ostringstream oss;
			oss << Err_Success << "|Successfully called" << Split_Record;
			int min = i * 10;
			int max = (i + 1) * 10;
			for (int j = min; j < max; j++) {
				if (j >= total) break;
				oss << GetTradeStr(trades[j]);
				if (j < total - 1) {
					oss << Split_Record;
				}
			}
			std::string formattedString = oss.str();
			session->Send(formattedString, msg_id);
			_LOG_INFO(LogicSyc::s_Logger, formattedString);

		}
	}
	catch (std::exception& e) {
		_LOG_INFO(LogicSyc::s_Logger, std::string("Exception code : ") + e.what());
	}
}

void LogicSyc::TradeTransaction(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	TradeTransInfo info = { 0 };
	memset(&info, 0, sizeof(TradeTransInfo));
	info.type = TT_BR_BALANCE;
	info.cmd = OP_BALANCE;//OP_BALANCE,OP_CREDIT
	info.orderby = 123123; // login
	info.price = 100; // balance;
	strncpy_s(info.comment, "web_dp", sizeof(info.comment));
	char errmsg[200] = "";
	int ret = s_mt4MgrServer->TradeTransaction(&info,errmsg);
	std::ostringstream oss;
	if (ret == RET_OK) {
		oss << Err_Success << "|Call succeeded";
	}
	else {
		oss << ret << errmsg;
	}
	
	std::string formattedString = oss.str();
	session->Send(formattedString, msg_id);
}

void LogicSyc::UserRecordNew(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	UserRecord user = { 0 };
	char errmsg[200] = "";
	int ret = s_mt4MgrServer->UserRecordNew(&user,errmsg);
	std::ostringstream oss;
	if (ret == RET_OK) {
		oss << Err_Success << "|Call succeeded";
	}
	else {
		oss << ret << errmsg;
	}

	std::string formattedString = oss.str();
	session->Send(formattedString, msg_id);
}

void LogicSyc::UserRecordUpdate(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	UserRecord user = { 0 };
	char errmsg[200] = "";
	int ret = s_mt4MgrServer->UserRecordUpdate(&user,errmsg);
	std::ostringstream oss;
	if (ret == RET_OK) {
		oss << Err_Success << "|Call succeeded ";
	}
	else {
		oss << ret << errmsg;
	}

	std::string formattedString = oss.str();
	session->Send(formattedString, msg_id);
}

void LogicSyc::GetUsersInfo(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {

	_LOG_INFO(LogicSyc::s_Logger, std::string("GetUsersInfo - msg data: ") + msg_data);
	
	int total = 0;
	try {

		UserRecord* users = s_mt4MgrServer->UsersRequest(&total);
		if (users != nullptr) {

			_LOG_INFO(LogicSyc::s_Logger, std::string("GetUsersInfo - user total : ") + std::to_string(total));
			int result = std::ceil(static_cast<double>(total) / 10);
			for (int i = 0;i < result; i++) {
				std::ostringstream oss;
				oss << Err_Success << "|Successfully called" << Split_Record;
				int min = i * 10;
				int max = (i + 1) * 10;
				for (int j = min; j < max; j++) {
					if (j >= total) break;
					oss << UserRecordToString(users[j]);
					if (j < total - 1) {
						oss << Split_Record;
					}
				}
				std::string formattedString = oss.str();
				session->Send(formattedString, msg_id);
				_LOG_INFO(LogicSyc::s_Logger, formattedString);
			}
			s_mt4MgrServer->MemFree(users);
		}
	}
	catch (std::exception& e) {
		_LOG_INFO(LogicSyc::s_Logger, std::string("Exception code : ") + e.what());
	}
}

void LogicSyc::PumpTrades(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {

	Mt4PumpSvr::GetInstance()->PumpTrades(session, msg_id, msg_data);
}

std::string LogicSyc::UserRecordToString(const UserRecord& user) {
    std::ostringstream oss;

    // Integer and string fields 
    oss << user.login << Split_Field
        << user.group << Split_Field
        // << user.password << Split_Field
        << user.enable << Split_Field
        << user.enable_change_password << Split_Field
        << user.enable_read_only << Split_Field
        << user.enable_otp << Split_Field
		<< user.enable_reserved << Split_Field
        // << user.password_investor << Split_Field
        // << user.password_phone << Split_Field
        << user.name << Split_Field
        << user.country << Split_Field
        << user.city << Split_Field
        << user.state << Split_Field
        << user.zipcode << Split_Field
        << user.address << Split_Field
        << user.lead_source << Split_Field
        << user.phone << Split_Field
        << user.email << Split_Field
        << user.comment << Split_Field
        << user.id << Split_Field
        << user.status << Split_Field
		<<  user.regdate << Split_Field
		<<  user.lastdate << Split_Field;

    // Financial Column
    oss << user.leverage << Split_Field
        << user.agent_account << Split_Field
        << user.timestamp << Split_Field
		<< user.last_ip << Split_Field
        << user.balance << Split_Field
        << user.prevmonthbalance << Split_Field
        << user.prevbalance << Split_Field
        << user.credit << Split_Field
        << user.interestrate << Split_Field
        << user.taxes << Split_Field
        << user.prevmonthequity << Split_Field
        << user.prevequity << Split_Field;

    // The remaining fields
	oss << user.reserved2 << Split_Field
    	<< user.otp_secret << Split_Field
		<< user.secure_reserved << Split_Field // secure data reserved
        << user.send_reports << Split_Field
        << user.mqid << Split_Field
        << user.user_color << Split_Field
		<< user.unused << Split_Field                // for future use
        << user.api_data;

    return oss.str();
}