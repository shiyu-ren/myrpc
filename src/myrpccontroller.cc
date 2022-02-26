#include "myrpccontroller.h"

MyRpcController::MyRpcController()
{
    m_failed = false;
    m_errText = "";
}

void MyRpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}
bool MyRpcController::Failed() const
{
    return m_failed;
}
std::string MyRpcController::ErrorText() const
{
    return m_errText;
}
void MyRpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}

    //目前未实现功能
void MyRpcController::StartCancel(){}
bool MyRpcController::IsCanceled() const{}
void MyRpcController::NotifyOnCancel(google::protobuf::Closure* callback){}