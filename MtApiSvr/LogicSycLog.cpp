#include "LogicSycLog.h"
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
//#include <boost/log/expressions/formatters/attr.hpp>
#include <boost/filesystem.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace src = boost::log::sources;

LogicSycLog::LogicSycLog() {}

LogicSycLog& LogicSycLog::get() {
    static LogicSycLog instance;
    return instance;
}

void LogicSycLog::init() {
    // Generate timestamped file name
    std::ostringstream oss;
    std::time_t t = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    oss << "logs/log_" << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".log";

    // Create logs folder if not exists
    boost::filesystem::create_directories("logs");

    logging::add_file_log(
        keywords::file_name = oss.str(),
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = logging::sinks::file::rotation_at_time_interval(boost::posix_time::hours(1)),
        keywords::format = (
            expr::stream
            << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
            << "] [" << logging::trivial::severity << "] " << expr::smessage
            )
    );

    logging::add_common_attributes();
}

void LogicSycLog::info(const std::string& msg) {
    BOOST_LOG_TRIVIAL(info) << msg;
}

void LogicSycLog::warn(const std::string& msg) {
    BOOST_LOG_TRIVIAL(warning) << msg;
}

void LogicSycLog::error(const std::string& msg) {
    BOOST_LOG_TRIVIAL(error) << msg;
}

void LogicSycLog::debug(const std::string& msg) {
    BOOST_LOG_TRIVIAL(debug) << msg;
}

