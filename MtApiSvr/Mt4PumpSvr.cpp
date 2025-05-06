#include "Mt4PumpSvr.h"
#include "Log.h"
#include "errcode.h"
#include "util.h"

#pragma comment(lib, "ws2_32.lib")

CManagerInterface* m_pumping;
extern CManagerFactory   m_factory;

std::vector<shared_ptr<CSession>> _pump_que;

volatile UINT      ExtPumpingMsg = 0;

//void __stdcall OnPumpingExFunc(int code, int type, void* data, void* param);

Mt4PumpSvr::Mt4PumpSvr(){
	
	m_svraddr = "wudc.hostdc.space:443";
	m_user = 503;
	m_pwd = "ca6cjew";

	m_log = std::make_shared<Log>();
	m_log->init("Mt4PumpSvrLog");

}
Mt4PumpSvr::~Mt4PumpSvr() {
	m_log = nullptr;
}

void Mt4PumpSvr::FreePump() {
	m_pumping->Release();
	WSACleanup();
}

bool Mt4PumpSvr::Init()
{
	LOG_INFO(m_log, std::string("Mt4PumpSvr::Init()"));
	m_factory.Init();
	m_factory.WinsockStartup();

	if (m_factory.IsValid() == false
		|| (m_pumping = m_factory.Create(ManAPIVersion)) == NULL)
	{
		printf("Init mt4 interface error\n");
		return false;
	}
	ExtPumpingMsg = RegisterWindowMessage("MetaTrader4_Pumping_Message");

	int res = RET_OK;
	res = m_pumping->Connect(m_svraddr.c_str());
	if (res != RET_OK) {
		LOG_INFO(m_log, std::string("Connect MT4 Svr Err:").append(m_pumping->ErrorDescription(res)));
		//m_log.Out("Connect Failed:%s", m_manager->ErrorDescription(res));
		return false;
	}
	res = m_pumping->Login(m_user, m_pwd.c_str());
	if (res != RET_OK)
	{
		LOG_INFO(m_log, std::string("Login MT4 Svr Err:").append(m_pumping->ErrorDescription(res)));
		//m_log.Out("Login Failed:%s", m_manager->ErrorDescription(res));
		return false;
	}

	res = m_pumping->PumpingSwitchEx(Mt4PumpSvr::OnPumpingExFunc, 0, this);
	if (res != RET_OK)
	{
		LOG_INFO(m_log, std::string("Pumping Faile:").append(m_pumping->ErrorDescription(res)));
		return false;
	}

	LOG_INFO(m_log, std::string("Pumping Start Success."));
	return true;
}

void __stdcall Mt4PumpSvr::OnPumpingExFunc(int code, int type, void* data, void* param)
{
	switch (code)
	{
	case PUMP_START_PUMPING:
		((Mt4PumpSvr*)param)->OnPumpStart(code,type,data);
		return;
		//return OnPumpStart();
		break;
	case PUMP_STOP_PUMPING:
		return ((Mt4PumpSvr*)param)->OnPumpStop();
		break;
	case PUMP_UPDATE_BIDASK:
		//return OnPumpBidAsk(code, type, data);
		break;
	case PUMP_UPDATE_SYMBOLS:

		break;
	case PUMP_UPDATE_GROUPS:

		break;
	case PUMP_UPDATE_USERS:
		/*UserRecord* puser = (UserRecord*)data;
		if (puser != NULL) {
			char msg[100];
			_snprintf_s(msg,sizeof(msg), "up_users. code %d login: %d  type: %d", code, puser->login, type);
			LOG_INFO(msg);
		}
		else {
			char msg[100];
			_snprintf_s(msg,sizeof(msg), "up_users. code %d data is null type:%d", code, type);
			LOG_INFO(msg);
		}*/
		return ((Mt4PumpSvr*)param)->OnUserUpdate(code, type, data);
		
		//return OnPumpUserRecord(code, type, data);
		break;
	case PUMP_UPDATE_ONLINE:
		//if (data != NULL)
		//{
		//	int login = *(int*)data;
		//	char msg[100];
		//	_snprintf_s(msg,sizeof(msg), "up_online. code %d login: %d  type: %d", code, login, type);
		//	LOG_INFO(msg);
		//	//OnlineRecord onlineRec;
		//	//if (RET_OK == m_manager->OnlineRecordGet(login, &onlineRec))
		//}
		//else {
		//	char msg[100];
		//	_snprintf_s(msg, sizeof(msg), "up_online. code %d data is null type:%d", code, type);
		//	LOG_INFO(msg);
		//}
		//return;
		//return OnPumpOnline(code, type, data);
		break;
	case PUMP_UPDATE_TRADES:
		/*TradeRecord* porder = (TradeRecord*)data;
		if (porder != NULL) {
			char msg[100];
			_snprintf_s(msg,sizeof(msg), "up_trade. code %d login: %d  type: %d", code, porder->order, type);
			LOG_INFO(msg);
		}
		else {
			char msg[100];
			_snprintf_s(msg,sizeof(msg), "up_trade. code %d data is null type:%d", code, type);
			LOG_INFO(msg);
		}
		return;*/
		//mtsvr.OnTradeUpdate();
		//return OnPumpTrades(code, type, data);
		return ((Mt4PumpSvr*)param)->OnTradeUpdate(code, type, data);
		break;
	case PUMP_UPDATE_ACTIVATION:

		break;
	case PUMP_UPDATE_MARGINCALL:

		break;
	case PUMP_UPDATE_REQUESTS:

		break;
	case PUMP_UPDATE_PLUGINS:

		break;
	case PUMP_UPDATE_NEWS:

		break;
	case PUMP_UPDATE_MAIL:

		break;

	case PUMP_PING:

		break;
	default: break;
	}
}

bool Mt4PumpSvr::Connect() {

	int res = RET_OK;
	res = m_pumping->Connect(m_svraddr.c_str());
	if (res != RET_OK) {
		LOG_INFO(m_log, std::string("Connect MT4 Service Err:").append(m_pumping->ErrorDescription(res)));
		//m_log.Out("Connect Failed:%s", m_manager->ErrorDescription(res));
		return false;
	}
	res = m_pumping->Login(m_user, m_pwd.c_str());
	if (res != RET_OK)
	{
		LOG_INFO(m_log, std::string("Login MT4 Service Err:").append(m_pumping->ErrorDescription(res)));
		//m_log.Out("Login Failed:%s", m_manager->ErrorDescription(res));
		return false;
	}

	return true;
}

void Mt4PumpSvr::CheckConnect() {
	if (m_pumping->Ping() != RET_OK)
	{
		LOG_INFO(m_log, std::string("ReConnect Pumping ."));
		Connect();

		int res = m_pumping->PumpingSwitchEx(OnPumpingExFunc, 0, this);
		if (res != RET_OK)
		{
			LOG_INFO(m_log, std::string("Pumping Switch Error:").append(m_pumping->ErrorDescription(res)));
			return;
		}
	}
}


void Mt4PumpSvr::OnPumpStart(int code, int type, void* data) {
	char msg[100];
	_snprintf_s(msg, sizeof(msg), "Pumping Start. code %d type: %d  ", code, type);
	LOG_INFO(m_log, msg);
	//LOG_INFO(std::string("Pumping��ʼ��"));
	//OnTradeUpdate();
	// WUPDATEACCOUNT MASTER = password | LOGIN = 65017309 | GROUP = 11WSC - S00 | CURRENCY = USD\r\nQUIT

	std::string errmsg;
	int userTotal = 1;
	//
	int in_login = 66000066;
	//UserRecord* pUsers = NULL;
	UserRecord pUser = {0};
	int res = m_pumping->UserRecordGet(in_login, &pUser);
	if (res != RET_OK) {
		errmsg = m_pumping->ErrorDescription(res);

	}
	////11WSC-E05
	//pUsers = m_pumping->UserRecordsRequest(&in_login, &userTotal);
	//memcpy(pUser.group, "11WSC-E04", sizeof(pUser.group));
	//
	//res = m_pumping->UserRecordUpdate(&pUser);
	//if (res != RET_OK) {
	//	errmsg = m_pumping->ErrorDescription(res);

	//}

	MarginLevel marginLevel = { 0 };
	//char t_group[200] = "11WSC-E04";
	
	res = m_pumping->MarginLevelGet(pUser.login, pUser.group, &marginLevel);
	if (res != RET_OK) {
		 errmsg = m_pumping->ErrorDescription(res);
		
	}

	ConGroup* m_group = NULL;
	int m_group_total = 0;
	m_group = m_pumping->GroupsGet(&m_group_total);
	
	_snprintf_s(msg, sizeof(msg), "--- group total : %d \0", m_group_total);
	LOG_INFO(m_log, msg);
	for (int i = 0;i < m_group_total;i++) {
		if (strcmp(m_group[i].group, pUser.group) == 0)
		{
			LOG_INFO(m_log, std::string(m_group[i].group));
		}

		LOG_INFO(m_log, std::string(m_group[i].group));
		_snprintf_s(msg, sizeof(msg), "default_leverage : %d \0", m_group[i].default_leverage);
		LOG_INFO(m_log, msg);
	}
	LOG_INFO(m_log, std::string("end group -------------------------"));
}
void Mt4PumpSvr::OnPumpStop() {
	LOG_INFO(m_log, std::string("Pumping End."));
}

//OP_NOTSET = 0,
//OP_INSERT,
//OP_UPDATE,
//OP_DELETE,
//OP_NONE,
void Mt4PumpSvr::OnUserUpdate(int code, int type, void* data) {
	UserRecord* puser = (UserRecord*)data;
	if (puser != NULL) {
		char msg[100];
		_snprintf_s(msg, sizeof(msg), "up_users. code %d login: %d  type: %d", code, puser->login, type);
		LOG_INFO(m_log, msg);
		if (type == OP_OPEN) {
			LOG_INFO(m_log, "Insert");
		}
		else if (type == OP_UPDATE) {
			LOG_INFO(m_log, "OP_UPDATE");
		}
		else if (type == OP_CLOSE) {
			LOG_INFO(m_log, "OP_DELETE");
		}
		else {
			LOG_INFO(m_log, "none");
		}
	}
	else {
		char msg[100];
		_snprintf_s(msg, sizeof(msg), "up_users. code %d data is null type:%d", code, type);
		LOG_INFO(m_log, msg);
	}
	return;
}

void Mt4PumpSvr::OnlineRecord(int code, int type, void* data) {
	if (data != NULL){
			int login = *(int*)data;
			char msg[100];
			_snprintf_s(msg,sizeof(msg), "up_online. code %d login: %d  type: %d", code, login, type);
			LOG_INFO(m_log, msg);
			//OnlineRecord onlineRec;
			//if (RET_OK == m_manager->OnlineRecordGet(login, &onlineRec))
		}
		else {
			char msg[100];
			_snprintf_s(msg, sizeof(msg), "up_online. code %d data is null type:%d", code, type);
			LOG_INFO(m_log, msg);
		}
}
//OP_NOTSET = 0,
//OP_INSERT,
//OP_UPDATE,
//OP_DELETE,
//OP_NONE,
void Mt4PumpSvr::OnTradeUpdate(int code, int type, void* data) {

	TradeRecord* porder = (TradeRecord*)data;
	if (porder != NULL) {
		
		char msg[100];
		_snprintf_s(msg, sizeof(msg), "up_trade. code %d login: %d  type: %d", code, porder->order, type);
		LOG_INFO(m_log, msg);
		if (type == OP_OPEN) {  // Closing Position
			LOG_INFO(m_log, "Open Order");
		}
		else if (type == OP_CLOSE) { // Opening Position
			LOG_INFO(m_log, "Close Order");
		}
		else if (type == OP_UPDATE) {
			LOG_INFO(m_log, "Modify Order");
		}
		else {
			LOG_INFO(m_log, "none");
		}
		if (_pump_que.size() > 0) {
			std::vector<int> intArray;
			int i = 0;
			for (std::vector<shared_ptr<CSession>>::iterator it = _pump_que.begin(); it != _pump_que.end(); ++it) {
				i++;
				shared_ptr<CSession> item = *it;
				bool isclose = item->isClose();
				if (isclose) {
					intArray.push_back(i);
					continue;
				}
				std::ostringstream oss;
				oss << Err_Success << "|Call successful" << Split_Record;
				oss << GetTradeStr(*porder);
				std::string formattedString = oss.str();
				item->Send(formattedString, 3002);
				//item->Send("200|update trade ", 3002);
			}
			// Remove closed links
			for (std::vector<int>::iterator it = intArray.begin(); it != intArray.end(); ++it) {
				int item = *it;
				_pump_que.erase(_pump_que.begin() + item - 1);
			}

		}
	}
	else {
		char msg[100];
		_snprintf_s(msg, sizeof(msg), "up_trade. code %d data is null type:%d", code, type);
		LOG_INFO(m_log, msg);
	}
	/*TradeRecord *m_trades = NULL;
	int m_trades_total = 0;
	m_trades = m_pumping->TradesGet(&m_trades_total);
	for (int i = 0; i < m_trades_total; i++)
	{
		TradeRecord& order = m_trades[i];
	}
	ConGroup* m_group = NULL;
	int m_group_total = 0;
	m_group = m_pumping->GroupsGet(&m_group_total);
	for (int i = 0;i < m_group_total;i++) {
		LOG_INFO(std::string(m_group[i].group));
	}*/
	//printf("����OnTradeUpdate \n");
	
	
//	LOG_INFO(std::string("TradesGetʧ�ܣ�"));
}

void Mt4PumpSvr::PumpTrades(shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	_pump_que.push_back(session);
	session->Send("200|Subscription successful", msg_id);
}

std::string Mt4PumpSvr::GetTradeStr(TradeRecord trade) {
	std::ostringstream tradeoss;
	std::string comment = util::GbkToUtf8(trade.comment);
	//std::cout << "comment: " << comment << endl;
	std::string splitStr = Split_Field;
	tradeoss << trade.order << Split_Field << trade.login << Split_Field << trade.symbol << Split_Field << trade.digits << Split_Field
		<< trade.cmd << Split_Field << trade.volume << Split_Field << trade.open_time << Split_Field << trade.open_price << Split_Field
		<< trade.sl << Split_Field << trade.tp << Split_Field << trade.close_time << Split_Field << trade.close_price << Split_Field
		<< trade.reason << Split_Field << trade.conv_rates[0] << Split_Field << trade.conv_rates[1] << Split_Field << comment << Split_Field
		<< trade.commission << Split_Field << trade.commission_agent << Split_Field << trade.storage << Split_Field << trade.profit << Split_Field
		<< trade.taxes << Split_Field << trade.margin_rate << Split_Field << trade.magic << Split_Field << trade.gw_volume << Split_Field
		<< trade.gw_open_price << Split_Field << trade.gw_close_price << Split_Field << trade.timestamp;
	return tradeoss.str();
}