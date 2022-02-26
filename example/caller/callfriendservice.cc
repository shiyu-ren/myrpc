#include "friend.pb.h"
#include "myrpcapplication.h"
#include <iostream>


int main(int argc, char **argv)
{
    //  整个程序启动以后，想使用myrpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MyRpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login 
    fdproto::FriendServiceRpc_Stub stub(new MyRpcChannel());
    //rpc方法的请求参数
    fdproto::GetFriendsListRequest request;
    request.set_userid(1000);
    //rpc方法的返回响应
    fdproto::GetFriendsListResponse response;

    MyRpcController controller;
    //发起rpc方法调用   同步的rpc调用过程  MyRpcChannel：：callmethod
    stub.GetFriendLists(&controller, &request, &response, nullptr);   //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    //一次调用结束，读取结果
    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {

        if(response.result().errcode() == 0)
        {
            std::cout << "rpc GetFriendsList success" << std::endl;
            int size = response.friends_size();
            for(int i = 0; i < size; ++i)
            {
                std::cout << "index:" << (i+1) << " name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response error:" << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}