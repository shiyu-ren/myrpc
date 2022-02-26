#include "myrpcprovider.h"
#include "myrpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

/*
service_name => service 描述
                                                =>  service* 记录服务的对象
                                                method_name => method方法对象

*/

void MyRpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象service的方法的数量
    int methodCnt = pserviceDesc->method_count();

    // std::cout << "service_name:" << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for(int i = 0; i < methodCnt; ++i)
    {
        //获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        // std::cout << "method_name:" << method_name << std::endl;
        LOG_INFO("method_name:%s", method_name.c_str());
        service_info.m_methodMap.insert({method_name, pmethodDesc});
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

void MyRpcProvider::Run()
{
    std::string ip = MyRpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    uint16_t port = atoi(MyRpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TCPserver对象
    muduo::net::TcpServer server(&m_eventLoop, address, "MyRpcProvider");
    // 绑定连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&MyRpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&MyRpcProvider::OnMessage, this,  std::placeholders::_1,  std::placeholders::_2,  std::placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    std::cout << "MyRpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    //启动网络服务
    server.start();
    m_eventLoop.loop();
}


void MyRpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
        std::cout << "Connection shutdown" << std::endl;
    }
    else
    {
        std::cout << "Connection Established" << std::endl;
    }
}

/*
    在框架内部，RpcProvider和RpcConsumer要协商好之间通信的protobuf格式
    service_name    method_name args    定义proto的message类型，进行数据头的序列化和反序列化
                                                                                    service_name    method_name     args_size
    head_size(4B) + header_str + args_str
*/

void MyRpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp timestamp)
{
    //  网络上接受的远程rpc调用请求的字符流     Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    //  从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    //  根据header_size读取数据头的原始字符流, 反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    myrpc::RpcHeader rpc_header;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpc_header.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpc_header.service_name();
        method_name = rpc_header.method_name();
        args_size = rpc_header.args_size();
    }
    else
    {
        //数据头反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
    }
    //  获取rpc方法参数的字符串
    std::string args_str = recv_buf.substr(4+header_size, args_size);

    //打印调试信息
    std::cout << "====================================" <<std:: endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_size << std::endl;
    std::cout << "====================================" <<std:: endl;

    //  获取service对象与method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }


    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end())
    {
        std::cout << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;      //获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second;     //获取method对象

    //  生成RPC方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //  给下面的method方法的调用，绑定一个Closure回调
    google::protobuf::Closure *done =  google::protobuf::NewCallback<MyRpcProvider, 
                                                                                                        const muduo::net::TcpConnectionPtr&, 
                                                                                                        google::protobuf::Message * >
                                                                                                        (this, &MyRpcProvider::SendRpcResponse, conn, response);

    //  在框架上根据远端rpc请求，调用当前rpc节点上的发布的方法
    //  new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}

void MyRpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    // 将response序列化
    std::string response_str;
    if(response->SerializeToString(&response_str))
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送回rpc的调用方
        std::cout << "MyRpcProvider send response_str: " << response_str << std::endl;
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response error!" << std::endl;
    }
    conn->shutdown();   //模拟http的短连接服务，由rpcprovider主动断开连接
}