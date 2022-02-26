#include "logger.h"
#include <time.h>
#include <iostream>

Logger* Logger::logger = nullptr;

Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread writeLogTask([&]()
        {
            for (;;)
            {
                //  获取当前的日期， 然后取日志信息，写入相应的日志文件当中 a+
                time_t now = time(nullptr);
                tm *nowtm = localtime(&now);

                std::string file_name = std::to_string(nowtm->tm_year+1900) + "-" +
                                                            std::to_string(nowtm->tm_mon+1) + "-" +
                                                            std::to_string(nowtm->tm_mday) + "-log.txt";
                FILE *pf = fopen(file_name.c_str(), "a+");
                if(pf == nullptr)
                {
                    std::cout << "logger file: " << file_name << "open error" << std::endl; 
                    exit(EXIT_FAILURE);
                }
                std::string msg;
                m_msgque.Pop(msg);
                char time_buf[128] = {0};
                sprintf(time_buf, "\ttime=%d:%d:%d [%s] \n ", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec, 
                                                                                                                (m_loglevel==INFO ? "info" : "error"));
                msg += time_buf;
                fputs(msg.c_str(), pf);
                fclose(pf);
            }
        }
    );

    // 设置分离线程， 守护线程
    writeLogTask.detach();
}

//设置Log级别
void Logger::SetLogLevel(const LogLevel& level)
{
    m_loglevel = level;
}
//  写日志, 把日志信息写入MsgQueue当中
void Logger::Log(const std::string& msg)
{
    m_msgque.Push(msg);
}
