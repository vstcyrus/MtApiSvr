#pragma once
#ifndef __LOG_H__
#define __LOG_H__

#include <log4cplus/initializer.h>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/log4cplus.h>

#include <iomanip>

#include <iostream>
#include <string>

using std::string;

class Log {
public:
    ///static Log* getInstance();
    //static Log* getLogicSycInstance();

    void error(const string& msg);
    void warn(const string& msg);
    void debug(const string& msg);
    void info(const string& msg);
    void print(const string& msg);

    void init(const char* prefixName);
    void LogicSycInit();
    Log();      

    //static Log* _Logger; 
    //static Log* _LogicSycLogger;
};

//#define postfix(msg) (std::string(msg).append(" [ ").append(__FILE__).append(":").append(__func__).append(":").append(std::to_string(__LINE__)).append(" ] "))

#define postfix(msg) (std::string(msg))

// Logging Macros
//#define LOG_ERROR(msg) Log::getInstance()->error(postfix(msg))
//#define LOG_WARN(msg)  Log::getInstance()->warn(postfix(msg))
//#define LOG_INFO(msg)  Log::getInstance()->info(postfix(msg))
//#define LOG_DEBUG(msg) Log::getInstance()->debug(postfix(msg))

#define LOG_ERROR(logger, msg) \
    do { \
        if (logger) logger->error(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)

#define LOG_WARN(logger, msg) \
    do { \
        if (logger) logger->warn(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)

#define LOG_INFO(logger, msg) \
    do { \
        if (logger) logger->info(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)

#define LOG_DEBUG(logger, msg) \
    do { \
        if (logger) logger->debug(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)



#define _LOG_ERROR(logger, msg) \
    do { \
        if (logger) logger->error(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)

#define _LOG_WARN(logger, msg) \
    do { \
        if (logger) logger->warn(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)

#define _LOG_INFO(logger, msg) \
    do { \
        if (logger) logger->info(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)

#define _LOG_DEBUG(logger, msg) \
    do { \
        if (logger) logger->debug(postfix(msg)); \
        std::cout << msg << std::endl; \
    } while (0)



#endif // LOG_H