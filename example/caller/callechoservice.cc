#include "echo.pb.h"
#include "myrpcapplication.h"
#include <iostream>
#include <mymuduo/Timestamp.h>

using namespace mymuduo;
static const int kRequests = 50000;

int main(int argc, char **argv)
{
    MyRpcApplication::Init(argc, argv);


    echo::EchoService_Stub stub(new MyRpcChannel());
    //rpc方法的请求参数
    echo::EchoRequest request;
    request.set_payload("001010");
    //rpc方法的返回响应
    echo::EchoResponse response;

    MyRpcController controller;

    Timestamp start(Timestamp::now());

    for (int i = 0; i < kRequests; ++i)
    {
        stub.Echo(&controller, &request, &response, nullptr);  
    }
    Timestamp end(Timestamp::now());

    double seconds = timeDifference(end, start);
    printf("%f seconds\n", seconds);
    printf("%.1f calls per second\n", kRequests / seconds);


    return 0;
}