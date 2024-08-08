#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
private:
    // zk客户端的句柄，可以通过句柄控制zkServer
    zhandle_t *m_zhandle;

public:
    ZkClient();
    ~ZkClient();
    // zkClient启动链接zkServer
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    void Create(const char *path, const char *data, int datalen, int state);
    // 在根据指定的path节点路径，获取数据
    std::string GetData(const char *path);
};
