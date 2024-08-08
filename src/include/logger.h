#pragma once
#include "lockqueue.h"

enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};

// mprpc框架提供的日志系统
class Logger
{
private:
    int m_logLevel;                  // 记录日志级别
    LockQueue<std::string> m_lckQue; // 日志缓冲队列

    // 单例模式
    Logger();
    // 以防偷偷使用拷贝构造函数
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;

public:
    // 获取单例实例
    static Logger &GetInstance();
    // 设置日志级别
    void SetLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);
};

// 定义宏

#define LOG_INFO(logmsgformat, ...)                     \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.SetLevel(INFO);                          \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0)

#define LOG_ERR(logmsgformat, ...)                      \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.SetLevel(ERROR);                         \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0)
