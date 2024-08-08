#include "mprpcconfig.h"

// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_str)
{
    int idx = src_str.find_first_not_of(' ');
    if (-1 != idx)
    {
        // 去掉字符串前面的空格
        src_str = src_str.substr(idx, src_str.size() - idx);
    }

    idx = src_str.find_last_not_of(' ');
    if (-1 != idx)
    {
        // 去掉字符串后面的空格
        src_str = src_str.substr(0, idx + 1);
    }
}

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *file = fopen(config_file, "r");
    if (nullptr == file)
    {
        std::cout << config_file << "is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 要处理的东西：1、注释，2、多余的空格 3、正确的配置项
    while (!feof(file))
    {
        char buff[512] = {0};
        fgets(buff, 512, file);
        std::string read_str(buff);
        // 去掉多余的空格
        Trim(read_str);
        // 判断#注释    如果是注释或者是空行
        if (read_str[0] == '#' || read_str.empty())
        {
            continue;
        }

        std::string key;
        std::string value;
        int idx = read_str.find("=");
        if (-1 == idx)
        {
            // 配置项不合法
            continue;
        }
        key = read_str.substr(0, idx);
        Trim(key);
        int endidx = read_str.find('\n', idx);
        value = read_str.substr(idx + 1, endidx - idx - 1);
        Trim(value);
        m_configMap.insert({key, value});
    }
}

// 查询配置项信息   <key,value>存储，所以应该传入的是key
std::string MprpcConfig::Load(const std::string &key)
{
    // return m_configMap[key]; 为什么不使用这个，因为如果key不存在，那么这个操作，会给map中直接添加这个key，并且value是空
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}