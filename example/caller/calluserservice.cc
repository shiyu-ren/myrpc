#include "user.pb.h"
#include "myrpcapplication.h"
#include <iostream>


int main(int argc, char **argv)
{
    //  整个程序启动以后，想使用myrpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MyRpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login 
    usproto::UserServiceRpc_Stub stub(new MyRpcChannel());
    //rpc方法的请求参数
    usproto::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //rpc方法的返回响应
    usproto::LoginResponse response;
    //发起rpc方法调用   同步的rpc调用过程  MyRpcChannel：：callmethod
    stub.Login(nullptr, &request, &response, nullptr);   //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    //一次调用结束，读取结果
    if(response.result().errcode() == 0)
    {
        std::cout << "rpc login response:" << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
    }

    //  演示调用远程发布的rpc方法Register
    usproto::RegisterRequest req;
    req.set_id(2000);
    req.set_name("myrpc");
    req.set_pwd("666666");

    usproto::RegisterResponse resp;
     stub.Register(nullptr, &req, &resp, nullptr);   //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    if(response.result().errcode() == 0)
    {
        std::cout << "rpc register response:" << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error:" << response.result().errmsg() << std::endl;
    }

    return 0;
}