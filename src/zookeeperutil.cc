#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <iostream>

// 全局的watcher观察器      zkServer给zkClient的通知
void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) // 回调的消息类型是和会话相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE) // 表示zkserver和zkclient连接成功
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh); // 在指定的句柄获取信号量
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}

// zkClient启动链接zkServer
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_ip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_port");
    std::string connstr = host + ":" + port; // init函数所需要的形式
    /*
    zookeeper_mt：多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程
    网络IO线程  poll
    watcher回调线程
    */

    // m_zhandle不是空指针，并不能说明zkserver和zkclient连接成功，只能说明创建句柄成功，
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem); // 给这个m_zkhandle添加了一个信号量，

    sem_wait(&sem); // 回调函数执行成功，这里sem被+1，表示获取到资源，才表示zkserver和zkclient连接成功
    std::cout << "zookeeper_init success!" << std::endl;
}

// 在zkserver上根据指定的path创建znode节点，state表示是临时节点还是会永久性节点，默认创建的是永久性节点
// 当心跳超时，临时性节点会被zkserver自动删除，永久性节点却不会
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buff[128] = {0};
    int bufflen = sizeof(path_buff);
    int flag;
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag) // 表示创建的节点不存在，所以我才创建新的节点,如果存在，我就不重复创建了
    {
        // 创建指定path的节点
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buff, bufflen);
        if (flag == ZOK) // 节点创建成功
        {
            std::cout << "znode create success... path:" << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error... path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 在根据指定的path节点路径，获取数据
std::string ZkClient::GetData(const char *path)
{
    char buff[1024] = {0};
    int bufflen = sizeof(buff);
    int flag = zoo_get(m_zhandle, path, 0, buff, &bufflen, nullptr); // 获取到的数据在这里被存在buff中了
    if (flag != ZOK)
    {
        std::cout << "get znode error... path:" << path << std::endl;
        return "";
    }
    else
    {
        return buff;
    }
}