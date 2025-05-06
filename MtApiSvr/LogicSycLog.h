#pragma once
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions.hpp>
#include <string>

class LogicSycLog {
public:
    static LogicSycLog& get(); // Singleton accessor

    void init(); // Initialize Boost.Log

    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void debug(const std::string& msg);

private:
    LogicSycLog();                         // Private constructor
    LogicSycLog(const LogicSycLog&) = delete;  // Delete copy
    LogicSycLog& operator=(const LogicSycLog&) = delete;
};


