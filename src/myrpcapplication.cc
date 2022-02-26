#include "myrpcapplication.h"
#include <iostream>
#include <unistd.h>

MyRpcConfig MyRpcApplication::m_config;

void ShowArgsHelp()
{
    std::cout << "format: command -i <config file>" << std::endl;
}

void MyRpcApplication::Init(int argc, char **argv)
{
    if(argc <  2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout << "invalid args!" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
            break;
        case ':':
            std::cout << "need config!" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
            break;
        default:
            break;
        }
    }

    // 开始加载配置文件了   rpcserver_ip=   rpcserver_port=   zookeeper_ip=    zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserver_ip:" << m_config.Load("rpcserver_ip") << std::endl;
    // std::cout << "rpcserverp_port:" << m_config.Load("rpcserver_port") << std::endl;
    // std::cout << "zookeeper_ip:" << m_config.Load("zookeeper_ip") << std::endl;
    // std::cout << "zookeeper_port:" << m_config.Load("zookeeper_port") << std::endl;
}

MyRpcApplication& MyRpcApplication::GetInstance()
{
    static MyRpcApplication app;
    return app;
}

MyRpcConfig& MyRpcApplication::GetConfig()
{
    return m_config;
}