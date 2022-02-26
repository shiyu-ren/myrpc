#include  <iostream>
#include "user.pb.h"
#include "myrpcapplication.h"

class UserService : public usproto::UserServiceRpc  //使用在rpc服务的发布端（rpc服务提供者）
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service : login" << std::endl;
        std::cout << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }    

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service:register" << std::endl;
        std::cout <<"id:" << id << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }
    //重写基类UserServiceRpc的虚函数   下面这些方法都是框架直接调用的
    //1. caller   ====>    Login(LoginRequest) ==>  muduo ==> callee
    //2. caller ===> Login(LoginRequest)  ==>交到下面重写的这个Login方法
    void Login(::google::protobuf::RpcController* controller,
                        const ::usproto::LoginRequest* request,
                        ::usproto::LoginResponse* response,
                        ::google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数LoginRequest，业务获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_result = Login(name, pwd);   //做本地业务
        //把响应写入 
        usproto::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //执行回调操作     执行响应对象数据的序列化和网络放送（由框架完成）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                        const ::usproto::RegisterRequest* request,
                        ::usproto::RegisterResponse* response,
                        ::google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数LoginRequest，业务获取相应数据做本地业务
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        

        bool login_result = Register(id, name, pwd);   //做本地业务
        //把响应写入 
        usproto::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //执行回调操作     执行响应对象数据的序列化和网络放送（由框架完成）
        done->Run();
    }
};

int main(int argc, char **argv)
{
    //本地调用
    // UserService us;
    // us.Login("zhang san", "123245");

    //调用框架的初始化操作
    MyRpcApplication::Init(argc, argv);

    //  provider是一个rpc网络服务对象 把服务发布到rpc节点上
    MyRpcProvider provider;
    provider.NotifyService(new UserService());
    // provider.NotifyService(new ....);

    //启动一个rpc服务发布节点，  Run以后  进程进入阻塞，等待caller的远程rpc调用请求
    provider.Run();
    return 0;
}