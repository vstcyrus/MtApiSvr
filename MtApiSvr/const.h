#pragma once

#define MAX_LENGTH  1024*2
// Total length of header
#define HEAD_TOTAL_LEN 4
// Header ID length
#define HEAD_ID_LEN 2
// Header data length
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE  10000
#define MAX_SENDQUE 10000

#define Split_Field   "|"
#define Split_Record  "#*#"

enum MSG_IDS {
	MSG_HEART = 1000, // Heartbeat 心跳
	MSG_GET_TRADES = 1001, // Get transaction order  获取交易单
	MSG_GET_TRADESHISTORY = 1002, // Get historical transactions 获取历史交易
	MSG_BALANCE = 1003, // Deposit and withdrawal Deposit and withdrawal credit 存取款 存取信用
	MSG_NEWUSER = 1004, // New account  新开账户
	MSG_UPDATEUSER = 1005, // Update account 更新账户
	MSG_GET_USERS_INFO = 1006, // Get all users info 獲取所有使用者的資料
	MSG_Pump_TRADE = 3001 // Subscribe to push messages 订阅推送消息
};



