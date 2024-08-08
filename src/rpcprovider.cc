#include "rpcprovider.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

/*
我们根据服务名字，可以得到很多的service描述，一个描述里面有很多的服务对象，
一个服务对象也会有很多方法名字，一个方法名字对应一个方法对象

映射表的设计就是根据这个来的
service_name => service描述
                        =》 service*  记录服务对象
                                    method_name  => method 方法对象
*/

// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口(向rpc中注册自己的服务)
/*
框架本身并不会知道服务方有什么服务，所以我们的service不能服务于某一个具体的类，
但是这些类的实现，肯定都是基于google::protobuf::Service 生成的，
即都是Service的派生类，所以我们只要对Service进行设计，就可以满足服务方的开发的所有服务
*/
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 获取了服务对象(用户服务的类)的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务对象的名字
    std::string service_name = pserviceDesc->name();
    LOG_INFO("service_name:%s", service_name.c_str());
    // 获取服务对象的方法的数量
    int methodCnt = pserviceDesc->method_count();

    for (int i = 0; i < methodCnt; i++)
    {
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        LOG_INFO("method_name:%s", method_name.c_str());
    }

    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动一个rpc服务节点，开始提供RPC远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TCPserver对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定链接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout 我们设置的是30s,（在zookeeper里面设置的），zkclient 的网络io线程，会以1/3 timeout时间发送ping消息，作为心跳消息
    ZkClient zkCli;
    zkCli.Start();
    for (auto &it : m_serviceMap)
    {
        // service_name  /UserserviceRpc
        std::string service_path = "/" + it.first;
        zkCli.Create(service_path.c_str(), nullptr, 0, 0);
        for (auto &mp : it.second.m_methodMap)
        {
            // method_name  /UserserviceRpc/Login,创建节点的路径只能一级一级创建，这个节点存储的是房钱这个rpc服务节点主机ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示创建的节点是临时性节点，当主机断开之后，注册的服务就会被删掉
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    std::cout
        << "RpcProvider start service at:" << ip << ":" << port << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 表示与rpc client连接断开
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型

UserServiceLoginzhang san123456
// 这就是protobuf存储的string，UserService是类名，Login是函数名，
// zhang san123456是login的参数，因为protobuf是二进制存储，并且是紧挨着，由于我们需要将这个数据反序列化出来，我们就要想办法提取相应的字段

我们定义protobuf数据类型message是下面的这样,为了我们进行数据头的序列化和反序列化
service_name method_name  args

前两个参数组成“消息头”，我们在message前面加上消息头的大小，（即告诉message中哪一段放的是我们的消息头）
为了便于提取args我们在消息头里面也加上后面“参数的大小（即参数的长度）”
因此，消息头message包含的：service_name  method_name  args_size

16UserServiceLogin***zhang san123456
// 前面的16表示消息头的大小,***表示后面参数的大小

header_size(4个字节)+header_str+args_str
16(header_size)  UserServiceLogin***(header_str) zhang san123456(args_str)

10 "10"
1000 "1000"

std::string insert和copy方法
*/

// 已建立链接的读写消息回调，如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp)
{
    // 网络上接受的远程rpc调用请求的字符流    需要调用服务的函数名Login，函数所需要的参数args
    std::string recv_buff = buffer->retrieveAllAsString();

    // 从字符流中读取前四个字节的内容
    uint32_t header_size = 0;
    recv_buff.copy((char *)&header_size, 4, 0); // 这是先将header_size强制转换成char*,以便为了进行复制操作，（函数参数的限制）

    // 根据header_size读取数据头的原始字符流,反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buff.substr(4, header_size);
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    mprpc::RpcHeader rpcHeader;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << "prase error!" << std::endl;
        return;
    }

    //  获取rpc方法参数的字符数据
    std::string args_str;
    args_str = recv_buff.substr(4 + header_size, args_size);

    std::cout << "==============================" << std::endl;
    std::cout << "recv_buff:" << recv_buff << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_size:" << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "==============================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " is not exist" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;      // 获取service对象      new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象

    // 生成rpc方法调用的请求request和response参数(request只包含了调用函数所需要的参数)
    google::protobuf::Message *request = service->GetRequestPrototype(method).New(); // 生成request对象
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error,context:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New(); // 生成response对象

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // 下面这行代码相当于调用new UserService().Login(controller,request,response,done)
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) // response进行序列化，将序列化的结果存在response_str
    {
        // 进行序列化成功之后，通过网络把rpc方法执行的结果发送给会调用rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "Serialize response_str error!" << std::endl;
    }
    conn->shutdown(); // 无论是否序列化成功都需要断开连接
}