#pragma once
#include "nocopy.hpp"
#include <iostream>
#include <string>
#include <mutex>
#include "timestamp.hpp"

namespace ns_log {

// 日志级别枚举
enum LogLevels
{
    INFO  = 1,  // 普通信息  
    DEBUG = 2,  // 调试信息
    ERROR = 3,  // 错误信息
    FATAL = 4,  // 致命信息
};

// 默认日志级别（可通过 -DLOG_LEVEL=3 进行修改）
#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

// 日志类（单例模式）
class Logger : public nocopy
{
public:
    static Logger& GetInstance()
    {
        static Logger log;
        return log;
    }

    ~Logger() {}

    // 写日志
    void writeLog(int level, const std::string& message)
    {
        std::string prefix;
        switch (level)
        {
        case INFO:
            prefix = "[INFO] ";
            break;
        case DEBUG:
            prefix = "[DEBUG] ";
            break;
        case ERROR:
            prefix = "[ERROR] ";
            break;
        case FATAL:
            prefix = "[FATAL] ";
            break;
        default:
            return;
        }
        std::string time = Timestamp::now().TimestamptoString();
        std::cout << prefix << "[" << time << "]" << ": " << message << std::endl;
    }

private:
    Logger() {}
};

} // namespace ns_log

// **日志宏（LOG_LEVEL 低于当前设定级别的将被移除）**
#if LOG_LEVEL <= 1
#define LOG_INFO(...)
#else
#define LOG_INFO(Logmsgformat, ...) \
    do { \
        ns_log::Logger& logger = ns_log::Logger::GetInstance(); \
        char buf[1024]; \
        snprintf(buf, 1024, "[%s:%d] " Logmsgformat, __FILE__, __LINE__, ##__VA_ARGS__); \
        logger.writeLog(ns_log::INFO, buf); \
    } while (0)
#endif

#if LOG_LEVEL <= 2
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(Logmsgformat, ...) \
    do { \
        ns_log::Logger& logger = ns_log::Logger::GetInstance(); \
        char buf[1024]; \
        snprintf(buf, 1024, "[%s:%d] " Logmsgformat, __FILE__, __LINE__, ##__VA_ARGS__); \
        logger.writeLog(ns_log::DEBUG, buf); \
    } while (0)
#endif

#if LOG_LEVEL <= 3
#define LOG_ERROR(...)
#else
#define LOG_ERROR(Logmsgformat, ...) \
    do { \
        ns_log::Logger& logger = ns_log::Logger::GetInstance(); \
        char buf[1024]; \
        snprintf(buf, 1024, "[%s:%d] " Logmsgformat, __FILE__, __LINE__, ##__VA_ARGS__); \
        logger.writeLog(ns_log::ERROR, buf); \
    } while (0)
#endif

#define LOG_FATAL(Logmsgformat, ...) \
    do { \
        ns_log::Logger& logger = ns_log::Logger::GetInstance(); \
        char buf[1024]; \
        snprintf(buf, 1024, "[%s:%d] " Logmsgformat, __FILE__, __LINE__, ##__VA_ARGS__); \
        logger.writeLog(ns_log::FATAL, buf); \
        std::exit(1); \
    } while (0)
