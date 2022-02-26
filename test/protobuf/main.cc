#include <iostream>
#include "test.pb.h"

using namespace figbug;

int main()
{
    //定义对象，设置数据. 封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    //序列化数据 
    std::string send_str;
    if(req.SerializeToString(&send_str))
    {
        std::cout << send_str << std::endl;
    }

    //反序列化出来一个login请求对象
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}