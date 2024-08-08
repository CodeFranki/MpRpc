#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include "zookeeperutil.h"
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

/*
我们定义的messsage类型
header_size + servicee_name  method_name args_size +  args_str
*/

// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    // 根据method可以知道他是所属哪一个服务的
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("Serialize request error!");
        return;
    }

    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("Serialize rpcheader error!");
        return;
    }

    // 组织待发送的rpc请求的字符
    std::string send_rpc_str;

    // 它会从header_size的地址开始，读取接下来的4个字节，并将这些字节视为字符
    // （尽管实际上它们表示的是一个整数的二进制表示），然后创建一个包含这些字节的std::string对象。
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    std::cout << "==============================" << std::endl;
    std::cout << "send_rpc_str:" << send_rpc_str << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_size:" << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "==============================" << std::endl;

    // 使用TCP编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 读取配置文件(这里读取的是mprpc框架的ip和端口，我们启动了一个框架之后，就有程序自动获取了一个框架实例和其配置文件)
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());

    // rpc想要调用service_name上面的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkcli;
    zkcli.Start();

    // 例如：/UserService/Login
    std::string host_path = "/" + service_name + "/" + method_name;
    std::string method_data = zkcli.GetData(host_path.c_str());
    if (method_data == "")
    {
        controller->SetFailed(host_path + "is not exist!");
        return;
    }
    int idx = method_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(host_path + "address is invalid!");
        return;
    }
    std::string ip = method_data.substr(0, idx);
    uint16_t port = atoi(method_data.substr(idx + 1, method_data.size() - idx).c_str());
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc节点
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 发送rpc的请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收rpc请求的响应值
    char recv_buff[1024] = {0};
    uint32_t recv_size = 0; // 表示接收到的数据大小
    if (-1 == (recv_size = recv(clientfd, recv_buff, 1024, 0)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化rpc调用的响应数据
    if (!response->ParseFromArray(recv_buff, recv_size))
    {
        close(clientfd);
        char errtxt[2048] = {0};
        sprintf(errtxt, "Parse error! response_str: %s", recv_buff);
        controller->SetFailed(errtxt);
        return;
    }
    close(clientfd);
}