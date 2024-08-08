#include "friend.pb.h"
#include "mprpcapplication.h"
#include <iostream>

int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架来使用rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login方法   可以共用一个stub
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数
    fixbug::GetFriendListRequest request; // 我想要调用远程的方法，我需要传入相应的参数
    request.set_id(1000);

    MprpcController controller;
    // rpc方法的响应
    fixbug::GetFriendListResponse response;                         // 远端执行之后给我返回的响应
    stub.GetFriendLists(&controller, &request, &response, nullptr); // 这里实际上调用的是RpcChannel-》RpcChannel::CallMethod，因此我们需要集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成，读调用的结果
    /*
    如果在网络传输或者其他意想不到的地方发生错误，我们不能仅仅依靠响应的错误值就判断rpc过程就是正确的，
    controller就可以知道
    */
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc login response success!" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; i++)
            {
                std::cout << "index:" << (i + 1) << " name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}
