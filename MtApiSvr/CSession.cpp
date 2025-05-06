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
	//Add exception handling  �����쳣����
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
//			//�Ѿ��ƶ����ַ���
//			int copy_len = 0;
//			while (bytes_transferred > 0) {
//				if (!_b_head_parse) {
//					//�յ������ݲ���ͷ����С
//					if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN) {
//						memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
//						_recv_head_node->_cur_len += bytes_transferred;
//						::memset(_data, 0, MAX_LENGTH);
//						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
//						return;
//					}
//					//�յ������ݱ�ͷ����
//					//ͷ��ʣ��δ���Ƶĳ���
//					int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
//					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
//					//�����Ѵ����data���Ⱥ�ʣ��δ����ĳ���
//					copy_len += head_remain;
//					bytes_transferred -= head_remain;
//					//��ȡͷ��MSGID����
//					short msg_id = 0;
//					memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
//					//�����ֽ���ת��Ϊ�����ֽ���
//					msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
//					std::cout << "msg_id is " << msg_id << endl;
//					//id�Ƿ�
//					if (msg_id > 65000 || msg_id <1000) {
//						std::cout << "invalid msg_id is " << msg_id << endl;
//						_server->ClearSession(_uuid);
//						return;
//					}
//					short msg_len = 0;
//					memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
//					//�����ֽ���ת��Ϊ�����ֽ���
//					msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
//					std::cout << "msg_len is " << msg_len << endl;
//					//id�Ƿ�
//					if (msg_len > MAX_LENGTH) {
//						std::cout << "invalid data length is " << msg_len << endl;
//						_server->ClearSession(_uuid);
//						return;
//					}
//
//					_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
//
//					//��Ϣ�ĳ���С��ͷ���涨�ĳ��ȣ�˵������δ��ȫ�����Ƚ�������Ϣ�ŵ����սڵ���
//					if (bytes_transferred < msg_len) {
//						memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
//						_recv_msg_node->_cur_len += bytes_transferred;
//						::memset(_data, 0, MAX_LENGTH);
//						_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
//							std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
//						//ͷ���������
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
//					//�˴�����ϢͶ�ݵ��߼�������
//					LogicSyc::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
//
//					//������ѯʣ��δ��������
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
//				//�Ѿ�������ͷ���������ϴ�δ���������Ϣ����
//				//���յ������Բ���ʣ��δ�����
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
//				//�˴�����ϢͶ�ݵ��߼�������
//				LogicSyc::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
//
//				//������ѯʣ��δ��������
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

			//Get the header MSGID data  - ��ȡͷ��MSGID����
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
			// Convert network byte order to local byte order - �����ֽ���ת��Ϊ�����ֽ���
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;
			//illegal id - id�Ƿ�
			if (msg_id > 65000 || msg_id < 1000) {
				std::string msg = "invalid msg_id is " + msg_id;
				LOG_ERROR(m_log, msg);
				_server->ClearSession(_uuid);
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			// Convert network byte order to local byte order - �����ֽ���ת��Ϊ�����ֽ���
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//illegal id - id�Ƿ�
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

//��ȡ��������
void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

//��ȡָ���ֽ���
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len,
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
			if (ec) {
				// ���ִ��󣬵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				//���ȹ��˾͵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// û�д����ҳ��Ȳ����������ȡ
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
			//�˴�����ϢͶ�ݵ��߼�������
			LogicSyc::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//��������ͷ�������¼�
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