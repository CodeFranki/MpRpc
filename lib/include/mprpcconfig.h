#pragma once

#include <unordered_map>
#include <iostream>
#include <string>

// rpcserver_ip rpcserver_port  zookeeper_ip  zookeeper_port
// 框架读取配置文件
class MprpcConfig
{
private:
    std::unordered_map<std::string, std::string> m_configMap;

public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char *config_file);

    // 查询配置项信息   <key,value>存储，所以应该传入的是key
    std::string Load(const std::string &key);

    // 去掉字符串前后的空格
    void Trim(std::string &src_buf);
};
