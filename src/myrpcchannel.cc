#include "myrpcchannel.h"
#include "rpcheader.pb.h"
#include "myrpcapplication.h"
#include "zookeeperutil.h"
#include <string>
// #include <muduo/net/TcpClient.h>
// #include <muduo/net/EventLoop.h>
// #include <muduo/net/InetAddress.h>
// #include <muduo/net/TcpConnection.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

/*
header_size + service_name  method_name  args_size + args
*/
void MyRpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                    google::protobuf::Message* response, google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    //获取参数的序列化字符串长度args_size
    std::string args_str;
    uint32_t args_size = 0;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        // std::cout << "serialize request error!" << std::endl;
        controller->SetFailed("serialize request error!" );
        return;
    }

    //定义rpc的请求header
    myrpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);
    //序列化header
    std::string rpc_header_str;
    uint32_t header_size = 0;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        // std::cout << "serialize header error!" << std::endl;
        controller->SetFailed("serialize header error!" );
        return;
    }
    //  组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

        //打印调试信息
    std::cout << "====================================" <<std:: endl;
    std::cout << "send_rpc_str: " << send_rpc_str << std::endl;
    std::cout << "====================================" <<std:: endl;
    //Todo: 使用muduo做客户端
    // muduo::net::EventLoop loop;
    // muduo::net::TcpClient client(&loop, new muduo::net::InetAddress(ip, port));

    //使用tcp编程，完成rpc远程方法调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);   //此处可以定义clientfd的删除器（使用智能指针）
    if(-1 == clientfd)
    {
        // std::cout << "create socket error!  errno = " <<  errno << std::endl;
        controller->SetFailed("create socket error! errno = " +  std::to_string(errno));
        return;
    }

    // 读取配置文件rcpserver的信息
    // std::string ip = MyRpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    // uint16_t port = atoi(MyRpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());

        // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr.s_addr);

    if(-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        // std::cout << "connect error!    errno=" << errno << std::endl;
        controller->SetFailed("connect error! errno = " +  std::to_string(errno));
        close(clientfd);
        exit(EXIT_FAILURE);
    }

    if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        // std::cout << "send error!    errno=" << errno << std::endl;
        controller->SetFailed("send error!  errno = " +  std::to_string(errno));
        close(clientfd);
        // exit(EXIT_FAILURE);
        return;
    }

    char recv_buf[1024] = {0};
    int recv_size = 0;
    if(-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        // std::cout << "recv error!    errno=" << errno << std::endl;
        controller->SetFailed("recv error!  errno = " +  std::to_string(errno));
        close(clientfd);
        // exit(EXIT_FAILURE);
        return;
    }
    //  反序列化rpc调用的响应数据
    // std::string response_str(recv_buf, 0, recv_size);    //  bug出现问题，recv_buf中遇到\0后面的数据就存不下来了
    // if(!response->ParseFromString(response_str))
    //改用从Array中反序列化
    if(!response->ParseFromArray(recv_buf, recv_size))
    {
        // std::cout << "parse error!" <<  std::endl;
        controller->SetFailed("parse error!" );
        close(clientfd);
        // exit(EXIT_FAILURE);
        return;
    }
    close(clientfd);
}
