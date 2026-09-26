#include "net/EventLoop.h"
#include "net/Acceptor.h"
#include<iostream>

int main()
{

  try
  {
    EventLoop loop;
    Acceptor acceptor(&loop,8080);

    acceptor.setNewConnectionCallback(
      [](int fd)
      {
        std::cout<<"上层收到新链接fd="<<fd<<"\n";
      }
    );

    acceptor.listen();
    std::cout<<"进入主循环,Ctrl+C退出\n";
    loop.loop();
  }
  catch(const std::exception& e)
  {
    std::cerr<<"错误:"<<e.what()<<"\n";
    return 1;
  }

    return 0;
}