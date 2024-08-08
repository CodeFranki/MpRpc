#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
using namespace std;

/*
原来的本地服务提供了两个进程内的本地方法，
*/
// class UserService
// {
// public:
//     void Login(string name, string pwd)
//     {
//         cout << "doing local service: Login" << endl;
//         cout << "name:" << name << "pwd:" << pwd << endl;
//     }
// };

// 将上面的本地服务login变成rpc远程服务  一定不要忘记加命名空间！！！！！！
// 我们这个本质上还是在使用“框架的接口”，我们是负责处理事物的服务提供者
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端(rpc服务提供者)
{
public:
    bool Login(string name, string pwd)
    {
        // 这一部分应该是实际做业务的代码
        cout << "doing local service: Login" << endl;
        cout << "name:" << name << endl;
        cout << "pwd:" << pwd << endl;
        return true;
    }

    bool Register(uint32_t id, string name, string pwd)
    {
        // 这一部分应该是实际做业务的代码
        cout << "doing local service: Register" << endl;
        cout << "id:" << id << endl;
        cout << "name:" << name << endl;
        cout << "pwd:" << pwd << endl;
        return true;
    }

    /*
重写基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
1、远端的服务器端想要调用Login，由框架的caller将Login(LoginRequest)打包发给muduo，再交给callee，
2、callee根据收到的Login(LoginRequest),知道需要调用Login这个方法，把这个函数要做的东西交给下面这个重写的Login方法上了
*/
    void Register(::google::protobuf::RpcController *controller,
                  const ::fixbug::RegisterRequest *request,
                  ::fixbug::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        uint16_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 本地函数做本地业务
        bool register_result = Register(id, name, pwd);

        // 把响应写入 包括错误码、错误消息，返回值,(相当于将本地做业务的结果放入response，把response返回给框架)
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("Register错误了");
        response->set_success(register_result);

        // 执行回调操作     执行响应对象数据的序列化和网络发送(都是由框架来完成的)
        done->Run();
    }

    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 本地函数做本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入 包括错误码、错误消息，返回值,(相当于将本地做业务的结果放入response，把response返回给框架)
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(1);
        code->set_errmsg("Login错误了");
        response->set_success(login_result);

        // 执行回调操作     执行响应对象数据的序列化和网络发送(都是由框架来完成的)
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc节点，run以后，进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();
    return 0;
}