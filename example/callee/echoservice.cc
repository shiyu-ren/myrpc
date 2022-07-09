#include  <iostream>
#include "echo.pb.h"
#include "myrpcapplication.h"

class EchoServiceImpl : public echo::EchoService
{
 public:
  virtual void Echo(::google::protobuf::RpcController* controller,
                    const ::echo::EchoRequest* request,
                    ::echo::EchoResponse* response,
                    ::google::protobuf::Closure* done)
  {
    //LOG_INFO << "EchoServiceImpl::Solve";
    response->set_payload(request->payload());
    done->Run();
  }
};

int main(int argc, char **argv)
{

    MyRpcApplication::Init(argc, argv);

    MyRpcProvider provider;
    provider.NotifyService(new EchoServiceImpl());

    provider.Run();
    return 0;
}