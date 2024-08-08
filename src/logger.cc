#include "logger.h"
#include "lockqueue.h"
#include <time.h>

// 单例模式
Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread writeLogTask([&]()
                             {for (;;)
                             {
                                // 获取当前的日期，然后获取日志信息，写入相应的
                                time_t now = time(nullptr);
                                tm* nowtm=localtime(&now);
                                char file_name[128] = {0};
                                sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

                                FILE *pf = fopen(file_name, "a+");
                                if(pf == nullptr){
                                    std::cout << "logger file: " << file_name << "open error!" << std::endl;
                                    exit(EXIT_FAILURE);
                                }
                                std::string msg = m_lckQue.Pop();
                                char time_buf[128] = {0};
                                sprintf(time_buf, "%d-%d-%d => [%s] ", 
                                nowtm->tm_hour, 
                                nowtm->tm_min, 
                                nowtm->tm_sec,
                                (m_logLevel == INFO ? "info":"error"));
                                msg.insert(0, time_buf);
                                msg.append("\n");
                                fputs(msg.c_str(), pf);
                                fclose(pf);
                             } });
    writeLogTask.detach();
}

// 获取单例实例
Logger &Logger::GetInstance()
{
    static Logger log;
    return log;
}
// 设置日志级别
void Logger::SetLevel(LogLevel level)
{
    m_logLevel = level;
}
// 写日志,将信息写入缓冲区队列中
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}