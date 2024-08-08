#include "test.pb.h"
#include <iostream>
#include <string>
using namespace std;
using namespace fixbug;

int main()
{
    // LoginResponse rsp;           // 里面有一个成员message
    // ResultCode *rc = rsp.mutable_result();       // 需要先获取对象指针
    // rc->set_errcode(1);          // 再对对象进行设置
    // rc->set_errmsg("登陆处理失败");

    GetFriendListsRespense rsp; // 里面有一个成员message
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    User *user1 = rsp.add_friend_list(); // 里面有一个列表成员，一般都是add_*
    user1->set_name("zhang san");        // 设置成员属性
    user1->set_age(32);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(23);
    user2->set_sex(User::MAN);

    cout << "列表的好友数量：" << rsp.friend_list_size() << endl; // 获取列表元素数量

    cout << "第一个用户的名字：" << rsp.friend_list(0).name() << endl;
    cout << "第二个用户的性别：" << rsp.friend_list(1).sex() << endl;
    // 枚举类型打印是整形，需要重新用一个函数获取字符串
    // std::string getSexName(int sexValue)
    // {
    //     if (sexValue == User::MAN)
    //     {
    //         return "MAN";
    //     }
    //     else if (sexValue == User::WOMAN)
    //     {
    //         return "WOMAN";
    //     }
    //     return "Unknown";
    // }

    // int sexValue = rsp.friend_list(1).sex();
    // std::cout << "用户性别：" << getSexName(sexValue) << std::endl;

    return 0;
}

#if 0

int main1()
{
    LoginRequest req;
    req.set_name("zhangsan");
    req.set_pwd("123456");

    // 对象数据序列化 =》string
    string send_str;
    if (req.SerializeToString(&send_str))
    {
        cout << send_str << endl;
    }

    // 从send_str反序列化一个login请求对象
    LoginRequest req1;
    if (req1.ParseFromString(send_str))
    {
        cout << req1.name() << endl;
        cout << req1.pwd() << endl;
    }
    return 0;
}

#endif