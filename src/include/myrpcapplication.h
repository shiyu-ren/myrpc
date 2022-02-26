#pragma once

#include "myrpcconfig.h"
#include "myrpcchannel.h"
#include "myrpccontroller.h"
#include "myrpcprovider.h"

//rpc框架的初始化类
//使用单例模式进行设计
class MyRpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MyRpcConfig& GetConfig();
    static MyRpcApplication& GetInstance();
private:
    static MyRpcConfig m_config;

    MyRpcApplication(){}
    MyRpcApplication(const  MyRpcApplication&) = delete;
    MyRpcApplication(MyRpcApplication&&) = delete;
};