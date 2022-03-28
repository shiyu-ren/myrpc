#include  <iostream>
#include "friend.pb.h"
#include "myrpcapplication.h"
#include "logger.h"



class FriendService : public fdproto::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendLists(uint32_t userid)
    {
        std::cout << "Do GetFriendLists method!" << std::endl;
        std::vector<std::string> vec{"zhangsan", "lisi", "wangwu"};
        return vec;
    }

    void GetFriendLists(::google::protobuf::RpcController* controller,
                        const ::fdproto::GetFriendsListRequest* request,
                        ::fdproto::GetFriendsListResponse* response,
                        ::google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数LoginRequest，业务获取相应数据做本地业务
        uint32_t userid = request->userid();

        std::vector<std::string> friendLists = GetFriendLists(userid);   //做本地业务
        //把响应写入 
        fdproto::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        for(auto item :friendLists)
        {
            response->add_friends(item.c_str());
        }

        //执行回调操作     执行响应对象数据的序列化和网络放送（由框架完成）
        done->Run();
    }

};


int main(int argc, char **argv)
{
    INFO_LOG("first log message!");
    ERR_LOG("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    //调用框架的初始化操作
    MyRpcApplication::Init(argc, argv);

    //  provider是一个rpc网络服务对象 把服务发布到rpc节点上
    MyRpcProvider provider;
    provider.NotifyService(new FriendService());
    // provider.NotifyService(new ....);

    //启动一个rpc服务发布节点，  Run以后  进程进入阻塞，等待caller的远程rpc调用请求
    provider.Run();
    return 0;
}