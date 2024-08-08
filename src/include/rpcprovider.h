#pragma once
#include "google/protobuf/service.h"
#include "mprpcapplication.h"

#include <string>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <unordered_map>
#include "google/protobuf/descriptor.h"

// 框架提供的专门发布rpc服务的网络对象类

class RpcProvider
{
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    // 由于服务提供端的类实现是基于google::protobuf::Service，即他们都是这个类的派生类，
    void NotifyService(google::protobuf::Service *service);

    // 启动一个rpc服务节点，开始提供RPC远程网络调用服务
    void Run();

private:
    // 组合了Eventloop
    muduo::net::EventLoop m_eventLoop;

    // 新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr &);

    // 已建立链接用户的消息读写回调
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);

    // service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;                                                    // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap; // 保存服务
    };

    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);
};
