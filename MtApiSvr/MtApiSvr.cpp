// MtApiSvr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include"Log.h"
#include <iostream>
#include "CServer.h"
#include "Singleton.h"
#include "LogicSyc.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "Mt4PumpSvr.h"
#include "Mt4MgrSvr.h"


using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

void StartPumpThread();

int main()
{
	try {
		log4cplus::Initializer initializer;

		
		std::thread t(StartPumpThread);
		
		auto pool = AsioIOServicePool::GetInstance();
		boost::asio::io_context  io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool](auto, auto) {
			
			io_context.stop();
			pool->Stop();
			});
		CServer s(io_context, 20002);
		io_context.run();
		t.join();
	}
	catch (std::exception& e) {
		//LOG_INFO(std::string("启动异常， 错误信息：").append(e.what()));
		std::cerr << "Exception: " << e.what() << endl;
	}
}

void StartPumpThread() {

	Mt4MgrSvr::GetInstance()->Init();
	
	Mt4PumpSvr mtpump;
	mtpump.Init();
	for (;;) {
		Sleep(1000);
		mtpump.CheckConnect();
		//printf("Pumping switch \n");
	}

	mtpump.FreePump();
}


