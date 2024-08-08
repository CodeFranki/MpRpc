#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架来使用rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login方法   可以共用一个stub
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数
    fixbug::LoginRequest request; // 我想要调用远程的方法，我需要传入相应的参数
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc方法的响应
    fixbug::LoginResponse response;                    // 远端执行之后给我返回的响应
    stub.Login(nullptr, &request, &response, nullptr); // 这里实际上调用的是RpcChannel-》RpcChannel::CallMethod，因此我们需要集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读调用的结果
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response:" << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc方法Register方法
    fixbug::RegisterRequest req;

    // rpc方法的请求参数
    req.set_id(2);
    req.set_name("mprpc");
    req.set_pwd("666666");

    // rpc方法的响应
    fixbug::RegisterResponse rsp;                // 远端执行之后给我返回的响应
    stub.Register(nullptr, &req, &rsp, nullptr); // 这里实际上调用的是RpcChannel-》RpcChannel::CallMethod，因此我们需要集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读调用的结果
    if (0 == rsp.result().errcode())
    {
        std::cout << "rpc login response:" << rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error: " << rsp.result().errmsg() << std::endl;
    }

    return 0;
}
