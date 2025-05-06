#include "Mt4MgrSvr.h"
#include "Log.h"
#include "errcode.h"

#pragma comment(lib, "ws2_32.lib")

CManagerInterface* m_manager;
CManagerFactory   m_factory;

Mt4MgrSvr::Mt4MgrSvr() {

	/*
	m_svraddr = "wudc.hostdc.space:443";
	m_user = 503;
	m_pwd = "ca6cjew";
	*/


	m_svraddr = "wudemodc.hostdc.space:444";
	m_user = 60;
	m_pwd = "V1cytzg";


	Init();
	m_log = std::make_shared<Log>();
	m_log->init("Mt4MgrSvrLog");
}

Mt4MgrSvr::~Mt4MgrSvr(){
	m_manager->Release();
	m_log = nullptr;
}

bool Mt4MgrSvr::Init() {

	LOG_INFO(m_log, std::string("Mt4MgrSvr::Init()"));
	m_factory.Init();
	m_factory.WinsockStartup();

	if (m_factory.IsValid() == false
		|| (m_manager = m_factory.Create(ManAPIVersion)) == NULL)
	{
		printf("Init mt4 interface error\n");
		return false;
	}


	// test 
	Connect();

	std::string errmsg;
	int userTotal = 1;

	//int in_login = 65100615;
	UserRecord* pUsers = NULL;
	///*UserRecord pUser = { 0 };
	//int res = m_manager->UserRecordGet(65017309, &pUser);
	//if (res != RET_OK) {
	//	errmsg = m_manager->ErrorDescription(res);

	//}*/
	////11WSC-E05
	//pUsers = m_manager->UserRecordsRequest(&in_login, &userTotal);
	//memcpy(pUsers->group, "11WSC-S00", sizeof(pUsers->group));
	//

	//int res = m_manager->UserRecordUpdate(pUsers);
	//if (res != RET_OK) {
	//	errmsg = m_manager->ErrorDescription(res);

	//}

	//MarginLevel marginLevel = { 0 };
	//int res = m_manager->MarginLevelRequest(66000066, &marginLevel);
	//if (res != RET_OK) {
	//	errmsg = m_manager->ErrorDescription(res);
    //
	//}

	// ----------------------------------------------------------------------------------------

	/*
	int res;
	for (int i = 0; i < 100; ++i) {

		int in_login = 65100615;
		pUsers = m_manager->UserRecordsRequest(&in_login, &userTotal);
		std::string s = "cyrussam" + std::to_string(i);
		memcpy(pUsers->name, s.c_str(), sizeof(pUsers->name));

		if (!pUsers) {
			std::cout << "Failed to get user record!" << std::endl;
			return true;
		}

		res = m_manager->UserRecordUpdate(pUsers);
		if (res != RET_OK) {
			errmsg = m_manager->ErrorDescription(res);
			std::cout << errmsg << std::endl;
		}
		else {
			std::cout << "Success to modify user record!" << std::endl;
		}
	}
	*/


	/* //------------------------------------------------------------------------------------ -
	int  login = 200007;
	__time32_t from = 0;//static_cast<__time32_t>(time(nullptr) - 365 * 24 * 60 * 60); 
	__time32_t to = static_cast<__time32_t>(time(nullptr)); 
	int total = 0;

	std::cout << "login = " << login << std::endl;
	std::cout << "from  = " << from << std::endl;
	std::cout << "to    = " << to << std::endl;

	TradeRecord* trades = m_manager->TradesUserHistory(login, from, to, &total);

	std::cout << std::endl << "login Id : " << login;
	std::cout << std::endl << "Order total : " << total << std::endl;

	*/ //-------------------------------------------------------------------------------------
}

bool Mt4MgrSvr::Connect() {
	int res = RET_OK;
	res = m_manager->Connect(m_svraddr.c_str());
	if (res != RET_OK) {
		LOG_INFO(m_log, std::string("Failed to connect to MT4 service:").append(m_manager->ErrorDescription(res)));
		//m_log.Out("Connect Failed:%s", m_manager->ErrorDescription(res));
		return false;
	}
	res = m_manager->Login(m_user, m_pwd.c_str());
	if (res != RET_OK)
	{
		LOG_INFO(m_log, std::string("Failed to log in to MT4 service:").append(m_manager->ErrorDescription(res)));
		//m_log.Out("Login Failed:%s", m_manager->ErrorDescription(res));
		return false;
	}
	return true;
}

TradeRecord* Mt4MgrSvr::GetTrades(int *total) {
	// TradeRecord* __stdcall TradesRequest(int *total)    
	if (m_manager->Ping() != RET_OK) {
		Connect();
	}
	return m_manager->TradesRequest(total);
}

TradeRecord* Mt4MgrSvr::TradesUserHistory(const int login, const __time32_t from, const __time32_t to, int* total) {

	if (m_manager->Ping() != RET_OK) {
		Connect();
	}
	return m_manager->TradesUserHistory(login,from,to,total);
}
// TradeRecord* __stdcall AdmTradesRequest(LPCSTR group,const int open_only,int *total)=0;

TradeRecord* Mt4MgrSvr::AdmTradesRequest(char * group, int* total) {
	if (m_manager->Ping() != RET_OK) {
		Connect();
	}
	return m_manager->AdmTradesRequest(group,1, total);
}

// Deposit and withdrawal, deposit and withdrawal credit
int Mt4MgrSvr::TradeTransaction(TradeTransInfo* info, char *msg) {
	if (m_manager->Ping() != RET_OK) {
		bool isconn= Connect();
		if (!isconn) {
			char _msg[200] = "Connect MT4 Error ";
			strncpy_s(msg, sizeof(msg), _msg, sizeof(_msg));
			LOG_INFO(m_log, msg); // cyrus add
			return Err_ConnMT4;
		}
	}
	int res = m_manager->TradeTransaction(info);
	if (res != RET_OK) {
		std::string errmsg = m_manager->ErrorDescription(res);
		strncpy_s(msg, sizeof(msg), errmsg.c_str(), sizeof(errmsg.c_str()));
		LOG_INFO(m_log, msg); // cyrus add
		return res;
	}
	return Err_Success;
}

// Create a new user
int Mt4MgrSvr::UserRecordNew(UserRecord* user, char* msg) {
	if (m_manager->Ping() != RET_OK) {
		bool isconn = Connect();
		if (!isconn) {
			char _msg[200] = "Connect MT4 Error ";
			strncpy_s(msg, sizeof(msg), _msg, sizeof(_msg));
			LOG_INFO(m_log, msg); // cyrus add
			return Err_ConnMT4;
		}
	}
	int res = m_manager->UserRecordNew(user);
	if (res != RET_OK) {
		std::string errmsg = m_manager->ErrorDescription(res);
		//strncpy_s(msg, sizeof(msg), errmsg.c_str(), sizeof(errmsg.c_str())); cyrus remove
		strncpy_s(msg, 200, errmsg.c_str(), errmsg.length() + 1);  // Ensure null-terminated
		LOG_INFO(m_log, msg); // cyrus add
		return res;
	}

	return Err_Success;
}

// get all user info
UserRecord* Mt4MgrSvr::UsersRequest(int* total)
{
	if (m_manager->Ping() != RET_OK) {
		bool isconn = Connect();
		if (!isconn) {
			std::string msg = "Connect MT4 Error";
			LOG_INFO(m_log, msg);
			return nullptr;
		}
	}
	return  m_manager->UsersRequest(total);
}

// Update User
int Mt4MgrSvr::UserRecordUpdate(UserRecord* user, char* msg) {
	if (m_manager->Ping() != RET_OK) {
		bool isconn = Connect();
		if (!isconn) {
			char _msg[200] = "Connect MT4 Error ";
			//strncpy_s(msg, sizeof(msg), _msg, sizeof(_msg));  cyrus remove
			strncpy_s(msg, 200, _msg, strlen(_msg) + 1);  // Ensure null-terminated
			LOG_INFO(m_log, msg); // cyrus add
			return Err_ConnMT4;
		}
	}
	int userTotal = 1;
	UserRecord* pUsers = NULL;
	pUsers = m_manager->UserRecordsRequest(&user->login, &userTotal);
	if (pUsers != NULL)
	{
		//memcpy(&user, &pUsers[0], sizeof(UserRecord));
		m_manager->MemFree(pUsers);
	}

	int res = m_manager->UserRecordUpdate(user);
	if (res != RET_OK) {
		std::string errmsg = m_manager->ErrorDescription(res);
		//strncpy_s(msg, sizeof(msg), errmsg.c_str(), sizeof(errmsg.c_str())); cyrus remove
		strncpy_s(msg, 200, errmsg.c_str(), errmsg.length() + 1);  // Ensure null-terminated
		LOG_INFO(m_log, msg); // cyrus add
		return res;
	}

	return Err_Success;
}

void Mt4MgrSvr::MemFree(void* ptr) {
	m_manager->MemFree(ptr);
}