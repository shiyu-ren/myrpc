#pragma once 

#include <google/protobuf/service.h>

class MyRpcController : public google::protobuf::RpcController
{
public:
    MyRpcController();

    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    //目前未实现功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed;  //  RPC方法执行过程中的调用
    std::string m_errText;  //  RPC方法执行过程中的错误信息
};