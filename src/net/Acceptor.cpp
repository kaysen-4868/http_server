#include "net/Acceptor.h"
#include "net/EventLoop.h"

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<cerrno>
#include<cstring>
#include<stdexcept>
#include<iostream>
#include<sys/epoll.h>

Acceptor::Acceptor(EventLoop* loop,int port)
: loop_(loop)
, listen_fd_(-1)
, port_(port)
{
    if(loop_==nullptr)
    {
        throw std::runtime_error("Acceptor: EventLoop不能为空");
    }
}


Acceptor::~Acceptor()
{
    if(listen_fd_!=-1)
    {
        loop_->removed(listen_fd_);
        loop_->clearCallbacks(listen_fd_);
        ::close(listen_fd_);
        listen_fd_=-1;
    }
}

//===========设置新连接回调============
 
void Acceptor::setNewConnectionCallback(NewConnectionCallback cb)
{
    new_conn_cb_=std::move(cb);
}

//=============开始监听=================

void Acceptor::listen()
{
    //创建listen_fd（直接设置为非阻塞）
    listen_fd_=::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
    if(listen_fd_==-1)
    {
        throw std::runtime_error("socket创建失败:"+std::string(strerror(errno)));
    }


//允许端口复用，避免重启时“address already in use"
   int opt=1;
   if(::setsockopt(listen_fd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))==-1)
   {
    ::close(listen_fd_);
    listen_fd_=-1;
    throw std::runtime_error("setsockopt失败:"+std::string(strerror(errno)));
   }

   //绑定地址
   struct sockaddr_in addr;
   std::memset(&addr,0,sizeof(addr));
   addr.sin_family=AF_INET;
   addr.sin_addr.s_addr=INADDR_ANY;
   addr.sin_port=htons(port_);

   if(::bind(listen_fd_,(struct sockaddr*)&addr,sizeof(addr))==-1)
   {
    ::close(listen_fd_);
    listen_fd_=-1;
    throw std::runtime_error("bind失败:(port="+std::to_string(port_)+")"+std::string(strerror(errno)));
   }

   //开始监听
   if(::listen(listen_fd_,128)==-1)
   {
    ::close(listen_fd_);
    listen_fd_=-1;
    throw std::runtime_error("listen失败:"+std::string(strerror(errno)));
   }


   //注册进EventLoop:listen可读->handlleAccept
   loop_->addFd(listen_fd_, EPOLLIN);
   loop_->setReadCallback(listen_fd_,[this]()
{
    handleAccept();
});

std::cout<<"Acceptor:监听窗口"<<port_<<"(listen_fd="<<listen_fd_<<")\n";
}

//============处理新链接============

void Acceptor::handleAccept()
{
    //ET模式：一次事件要把所有待处理的连接全部acceppt完
    while(true)
    {
     struct sockaddr_in client_addr;
     socklen_t client_len=sizeof(client_addr);

     //使用accept4直接创建非阻塞fd(linux特有)
     int client_fd=::accept4(listen_fd_,
    (struct sockaddr*)&client_addr,&client_len,SOCK_NONBLOCK);

    if(client_fd==-1)
    {
        if(errno==EAGAIN||errno==EWOULDBLOCK)
        {
            break;
        }
        if(errno==EINTR)
        {
            continue;
        }
        if(errno==EMFILE||errno==ENFILE)
        {
            //达到进程fd上限
            std::cerr<<"accept失败:文件描述符耗尽（"<<strerror(errno)<<")\n";
            break;
        }

        std::cerr<<"accept失败:"<<strerror(errno)<<"\n";
        break;
    }
    //打印客户端信息
    char ip_str[INET_ADDRSTRLEN]={0};
    ::inet_ntop(AF_INET,&client_addr.sin_addr,ip_str,sizeof(ip_str));
    std::cout<<"新链接:"<<ip_str<<":"<<ntohs(client_addr.sin_port)<<"client_fd="<<client_fd<<"\n";

    //通知上层：有新连接了（把fd交出去，有上层创建connection
    if(new_conn_cb_)
    {
        new_conn_cb_(client_fd);
    }
    else
    {
        std::cerr<<"警告:未设置NewConnectionCallback,关闭client_fd="<<client_fd<<"\n";
        ::close(client_fd);
    }

    }
}



