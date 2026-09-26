#include "net/EventLoop.h"
#include "net/Acceptor.h"
#include "net/Connection.h"
#include<unordered_map>
#include<iostream>

int main()
{

  try
  {
    EventLoop loop;
    Acceptor acceptor(&loop,8080);

    //保存所有连接，防止shared_ptr被提前释放
    auto conns=std::make_shared<std::unordered_map<int,std::shared_ptr<Connection>>>();
    
    acceptor.setNewConnectionCallback([&loop,conns](int client_fd)
  {
    auto conn=std::make_shared<Connection>(&loop,client_fd);

    //echo 逻辑:把读到的数据原样发回去
    conn->setMessageCallback([](const std::shared_ptr<Connection>&c)
  {
    std::string& in=c->inputBuffer();
    if(!in.empty())
    {
      std::cout<<"收到"<<in.size()
      <<"字节，回显\n";
      c->send(in);
      in.clear();
    }
  });

  conn->setCloseCallback([conns](const std::shared_ptr<Connection>&c)
{
  conns->erase(c->fd());
  std::cout<<"连接已清理fd="<<c->fd()<<"\n";
});

(*conns)[client_fd]=conn;

  });

  acceptor.listen();
  std::cout<<"进入主循环,Ctrl+C 退出\n";
  loop.loop();

  }
  catch(const std::exception& e)
  {
    std::cerr<<"错误:"<<e.what()<<"\n";
    return 1;
  }

    return 0;
}