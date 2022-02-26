#pragma once

#include "msgqueue.hpp"

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

// myrpc框架提供的异步日志系统
enum LogLevel
{
    INFO,   //  普通信息
    ERROR,  //错误信息
};

class Logger
{
public:
    //设置Log级别
    void SetLogLevel(const LogLevel& level);
    //  写日志
    void Log(const std::string& msg);
    //  获取日志的单例
    static Logger& GetInstance()
    {
        if(logger == nullptr)
            logger = new Logger();
        return *logger;
    }

private:
    int m_loglevel; //  记录日志级别
    MsgQueue<std::string> m_msgque;   //  字符串日志缓冲队列
    static Logger *logger;
    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
};

