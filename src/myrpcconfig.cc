#include "myrpcconfig.h"
#include <iostream>

void MyRpcConfig::Trim(std::string& src_buf)
{
    int idx = src_buf.find_first_not_of(' ');

    if(idx != -1)
    {
        //说明字符串前面有空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    //  去掉字符串后面多余的空格
    idx = src_buf.find_last_not_of(' ');
    if(idx != -1)
    {
        //说明字符串后面有空格
        src_buf = src_buf.substr(0, idx+1);
    }

}

//  负责解析加载配置文件
void MyRpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if(pf == nullptr)
    {
        std::cout << config_file << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    //1. 注释   2.正确的配置项=     3.取出开头的多余的空格
    while(!feof(pf))
    {
        char buf[512];
        fgets(buf, 512, pf);

        //  去掉字符串前面多余的空格
        std::string read_buf(buf);
        Trim(read_buf);

        //判断#的注释
        if(read_buf[0] == '#'  || read_buf.empty())   continue;

        //  解析配置项
        int idx = read_buf.find('=');
        if(idx == -1)   continue;   //不合法

        std::string key, value;
        key = read_buf.substr(0, idx);
        Trim(key);

        int endidx = read_buf.find('\n', idx+1);
        value = read_buf.substr(idx+1, endidx-idx-1);
        Trim(value);
        m_configMap.insert({key, value});
    }
}
//  查询配置项信息
std::string MyRpcConfig::Load(const std::string& key)
{
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()) 
    {
        return "";
    }
    return it->second;
}