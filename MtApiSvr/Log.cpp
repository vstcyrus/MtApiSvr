#include "Log.h"
#include <exception>
#include <iostream>
#include <log4cplus/configurator.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;

//Log* Log::_Logger = nullptr;
//Log* Log::_LogicSycLogger = nullptr;

/*
Log* Log::getInstance() {
    if (_Logger == nullptr) {
        _Logger = new Log();
        _Logger->init();
    }
    return _Logger;
}

Log* Log::getLogicSycInstance() {
    if (_LogicSycLogger == nullptr) {
        _LogicSycLogger = new Log();
        _LogicSycLogger->LogicSycInit();
    }
    return _LogicSycLogger;
}
*/

Log::Log() {
}

void Log::init(const char* prefixName) {
    try {

        PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("LogicSycLog.conf"));
        std::ostringstream oss;
        std::time_t t = std::time(nullptr);
        std::tm tm;
        localtime_s(&tm, &t);
        oss << "logs/" << prefixName << ".log";
        SharedAppenderPtr customAppender(new FileAppender(LOG4CPLUS_TEXT(oss.str())));
        customAppender->setLayout(std::unique_ptr<Layout>(new PatternLayout(LOG4CPLUS_TEXT("%d{%Y-%m-%d %H:%M:%S} [%t] %-5p %c - %m%n"))));

        Logger rootLogger = Logger::getRoot();
        rootLogger.removeAllAppenders();          // Optional: Clear old appenders
        rootLogger.addAppender(customAppender);   // Replace with timestamped appender


        LOG4CPLUS_INFO(rootLogger, LOG4CPLUS_TEXT("Logging initialized!"));
    }
    catch (...) {
        std::cerr << "Failed to initialize logger" << std::endl;
    }
}

void Log::LogicSycInit() {
    try {

        PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("LogicSycLog.conf"));
        std::ostringstream oss;
        std::time_t t = std::time(nullptr);
        std::tm tm;
        localtime_s(&tm, &t);
        oss << "logs/logicSycLog" << ".log";
        SharedAppenderPtr customAppender(new FileAppender(LOG4CPLUS_TEXT(oss.str())));
        customAppender->setLayout(std::unique_ptr<Layout>(new PatternLayout(LOG4CPLUS_TEXT("%d{%Y-%m-%d %H:%M:%S} [%t] %-5p %c - %m%n"))));

        Logger rootLogger = Logger::getRoot();
        rootLogger.removeAllAppenders();          // Optional: Clear old appenders
        rootLogger.addAppender(customAppender);   // Replace with timestamped appender


        LOG4CPLUS_INFO(rootLogger, LOG4CPLUS_TEXT("Logging initialized!"));
    }
    catch (...) {
        std::cerr << "Failed to initialize logger" << std::endl;
    }
}


void Log::error(const string& msg) {
    LOG4CPLUS_ERROR(log4cplus::Logger::getRoot(), LOG4CPLUS_STRING_TO_TSTRING(msg));
}

void Log::warn(const string& msg) {
    LOG4CPLUS_WARN(log4cplus::Logger::getRoot(), LOG4CPLUS_STRING_TO_TSTRING(msg));
}

void Log::debug(const string& msg) {
    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), LOG4CPLUS_STRING_TO_TSTRING(msg));
}

void Log::info(const string& msg) {
    LOG4CPLUS_INFO(log4cplus::Logger::getRoot(), LOG4CPLUS_STRING_TO_TSTRING(msg));
}

void Log::print(const string& msg) {
    std::cerr << msg << std::endl;
}
