#include "CSession.h"
#include "CServer.h"
#include <iostream>
#include <sstream>
//#include <json/json.h>
//#include <json/value.h>
//#include <json/reader.h>
#include "LogicSyc.h"
#include "Log.h"

CSession::CSession(boost::asio::io_context& io_context, CServer* server) :
	_socket(io_context), _server(server), _b_close(false), _b_head_parse(false) {

	m_log = std::make_shared<Log>();
	m_log->init("CSession");
	boost::uuids::uuid  a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
	::memset(_data, 0, MAX_LENGTH);
}
CSession::~CSession() {
	std::cout << "~CSession destruct" << endl;
}

tcp::socket& CSession::GetSocket() {
	return _socket;
}

std::string& CSession::GetUuid() {
	return _uuid;
}

void CSession::Start() {
	/*::memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), std::bind(&CSession::HandleRead, this,
		std::placeholders::_1, std::placeholders::_2, SharedSelf()));*/
	AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		// std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << endl;
		std::string msg = "session: " + _uuid + " send que fulled, size is " + std::to_string(MAX_SENDQUE);
		LOG_ERROR(m_log, msg);
		//return;
	}

	_send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Send(char* msg, short max_length, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::string msg = "session: " + _uuid + " send que fulled, size is " + std::to_string(MAX_SENDQUE);
		LOG_ERROR(m_log,msg);
		//return;
	}

	_send_que.push(make_shared<SendNode>(msg, max_length, msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close() {
	_socket.close();
	_b_close = true;
}
bool CSession::isClose() {
	return _b_close;
}

std::shared_ptr<CSession>CSession::SharedSelf() {
	return shared_from_this();
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self) {
	//Add exception handling  增加异常处理
	try {
		if (!error) {
			std::lock_guard<std::mutex> lock(_send_lock);
			//cout << "send data " << _send_que.front()->_data+HEAD_LENGTH << endl;
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			std::string msg = "handle write failed, error is " + error.what();
			LOG_ERROR(m_log, msg);
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (std::exception& e) {
		std::string msg = "Exception code : " + std::string(e.what());
		LOG_ERROR(m_log, msg);
	}

}

//void CSession::HandleRead(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<CSession> shared_self) {
//	try {
//		if (!error) {
//			//已经移动的字符数
//			int copy_len = 0;
//			while (bytes_transferred > 0) {
//				if (!_b_head_parse) {
//					//收到的数据不足头部大小
//					if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN) {
//						memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
//						_recv_head_node->_cur_len += bytes_transferred;
//						::memset(_data, 0, MAX_LENGTH);
//						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
//						return;
//					}
//					//收到的数据比头部多
//					//头部剩余未复制的长度
//					int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
//					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
//					//更新已处理的data长度和剩余未处理的长度
//					copy_len += head_remain;
//					bytes_transferred -= head_remain;
//					//获取头部MSGID数据
//					short msg_id = 0;
//					memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
//					//网络字节序转化为本地字节序
//					msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
//					std::cout << "msg_id is " << msg_id << endl;
//					//id非法
//					if (msg_id > 65000 || msg_id <1000) {
//						std::cout << "invalid msg_id is " << msg_id << endl;
//						_server->ClearSession(_uuid);
//						return;
//					}
//					short msg_len = 0;
//					memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
//					//网络字节序转化为本地字节序
//					msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
//					std::cout << "msg_len is " << msg_len << endl;
//					//id非法
//					if (msg_len > MAX_LENGTH) {
//						std::cout << "invalid data length is " << msg_len << endl;
//						_server->ClearSession(_uuid);
//						return;
//					}
//
//					_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
//
//					//消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
//					if (bytes_transferred < msg_len) {
//						memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
//						_recv_msg_node->_cur_len += bytes_transferred;
//						::memset(_data, 0, MAX_LENGTH);
//						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
//						//头部处理完成
//						_b_head_parse = true;
//						return;
//					}
//
//					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
//					_recv_msg_node->_cur_len += msg_len;
//					copy_len += msg_len;
//					bytes_transferred -= msg_len;
//					_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
//					//cout << "receive data is " << _recv_msg_node->_data << endl;
//					//此处将消息投递到逻辑队列中
//					LogicSyc::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
//
//					//继续轮询剩余未处理数据
//					_b_head_parse = false;
//					_recv_head_node->Clear();
//					if (bytes_transferred <= 0) {
//						::memset(_data, 0, MAX_LENGTH);
//						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
//						return;
//					}
//					continue;
//				}
//
//				//已经处理完头部，处理上次未接受完的消息数据
//				//接收的数据仍不足剩余未处理的
//				int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
//				if (bytes_transferred < remain_msg) {
//					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
//					_recv_msg_node->_cur_len += bytes_transferred;
//					::memset(_data, 0, MAX_LENGTH);
//					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//						std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
//					return;
//				}
//				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
//				_recv_msg_node->_cur_len += remain_msg;
//				bytes_transferred -= remain_msg;
//				copy_len += remain_msg;
//				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
//				//cout << "receive data is " << _recv_msg_node->_data << endl;
//				//此处将消息投递到逻辑队列中
//				LogicSyc::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
//
//				//继续轮询剩余未处理数据
//				_b_head_parse = false;
//				_recv_head_node->Clear();
//				if (bytes_transferred <= 0) {
//					::memset(_data, 0, MAX_LENGTH);
//					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//						std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
//					return;
//				}
//				continue;
//			}
//		}
//		else {
//			std::cout << "handle read failed, error is " << error.what() << endl;
//			Close();
//			_server->ClearSession(_uuid);
//		}
//	}
//	catch (std::exception& e) {
//		std::cout << "Exception code is " << e.what() << endl;
//	}
//}

void CSession::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::string msg = "handle read failed, error is " + ec.what();
				LOG_ERROR(m_log, msg);
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::string msg = "read length not match, read [" + std::to_string(bytes_transfered) + "] , total [" + std::to_string(HEAD_TOTAL_LEN) + "]";
				LOG_ERROR(m_log, msg);
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

			//Get the header MSGID data  - 获取头部MSGID数据
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
			// Convert network byte order to local byte order - 网络字节序转化为本地字节序
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;
			//illegal id - id非法
			if (msg_id > 65000 || msg_id < 1000) {
				std::string msg = "invalid msg_id is " + msg_id;
				LOG_ERROR(m_log, msg);
				_server->ClearSession(_uuid);
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			// Convert network byte order to local byte order - 网络字节序转化为本地字节序
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//illegal id - id非法
			if (msg_len > MAX_LENGTH) {
				std::string msg = "invalid data length is " + msg_len;
				LOG_ERROR(m_log, msg);
				_server->ClearSession(_uuid);
				return;
			}

			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::string msg = "Exception code is " + std::string(e.what());
			LOG_ERROR(m_log, msg);
		}
		});
}

//读取完整长度
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

//读取指定字节数
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len,
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
			if (ec) {
				// 出现错误，调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				//长度够了就调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// 没有错误，且长度不足则继续读取
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
		});
}

void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::string msg = "handle read failed, error is " + ec.what();
				LOG_ERROR(m_log, msg);
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			if (bytes_transfered < total_len) {
				std::string msg = "read length not match, read [" + std::to_string(bytes_transfered) + "] , total [" + std::to_string(total_len) + "]";
				LOG_ERROR(m_log, msg);
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			memcpy(_recv_msg_node->_data, _data, bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << _recv_msg_node->_data << endl;
			//此处将消息投递到逻辑队列中
			LogicSyc::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//继续监听头部接受事件
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::string msg = "Exception code is " + std::string(e.what());
			LOG_ERROR(m_log, msg);
		}
		});
}

LogicNode::LogicNode(shared_ptr<CSession>  session,
	shared_ptr<RecvNode> recvnode) :_session(session), _recvnode(recvnode) {

}