#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <mymuduo/TcpServer.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>
#include <mymuduo/TcpConnection.h>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

//框架提供的专门服务发布rpc服务的网络对象类
class MyRpcProvider
{
public:
    //  这里是框架提供外部使用的，通用的发布rpc服务的接口。  注意要使用Service，因为服务都继承在Service
    void NotifyService(google::protobuf::Service *service);

    //启动rpc服务节点， 开始提供rpc远程调用服务
    void Run();

private:
    //组合了EventLoop
    mymuduo::EventLoop m_eventLoop;

    struct ServiceInfo
    {
        google::protobuf::Service *m_service;                                                    //保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap; //保存服务方法
    };

    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    void OnConnection(const mymuduo::TcpConnectionPtr &conn);
    void OnMessage(const mymuduo::TcpConnectionPtr &conn, mymuduo::Buffer *buffer, mymuduo::Timestamp timestamp);

    //  Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const mymuduo::TcpConnectionPtr &, google::protobuf::Message *);
};