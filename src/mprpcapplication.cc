#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config; // 静态成员函数想要访问静态成员变量，变量需要在类外进行初始化

void showArgsHelp()
{
    std::cout << "format:commmad -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv)
{
    if (argc < 2)
    {
        showArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1)
    { // i：表示必须有i这个参数,如果选项字符后面跟着一个冒号（:），则表示该选项需要一个附加的参数。
        switch (c)
        {
        case 'i': // 表示参数是我们想要的-i
            config_file = optarg;
            break;
        case '?': // 表示参数是我们不想要的i，而是其他的符号
            showArgsHelp();
            exit(EXIT_FAILURE);
        case ':': // 表示参数里面确实有-i,但是-i后面没有带参数
            showArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 开始加载配置文件 rpcserver_ip  rpcserver_port zoo
    m_config.LoadConfigFile(config_file.c_str());
}

MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig &MprpcApplication::GetConfig()
{
    return m_config;
}