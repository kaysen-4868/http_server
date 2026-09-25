#include"net/EventLoop.h"

#include<sys/epoll.h>
#include<unistd.h>
#include<cerrno>
#include<cstring>
#include<stdexcept>
#include<iostream>

//构造函数
EventLoop::EventLoop()
:epoll_fd_(-1),
running_(false)
{
    epoll_fd_=epoll_create1(0);
    if(epoll_fd_==-1)
    {
        throw std::runtime_error("epoll_create1失败:"+std::string(strerror(errno)));
    }
}
//析构函数
EventLoop::~EventLoop()
{
    if(epoll_fd_!=-1)
    {
        ::close(epoll_fd_);
        epoll_fd_=-1;
    }
}

void EventLoop::loop()
{
    running_=true;
    struct epoll_event events[MAX_EVENTS];

    while(running_)
    {
     int nfds=epoll_wait(epoll_fd_,events,MAX_EVENTS,-1);

     if(nfds==-1)
     {
        if(errno==EINTR)
        {
            continue;
        }
        std::cerr<<"epoll_wait失败:"<<strerror(errno)<<"\n";
        break;
     }

     for(int i=0;i<nfds;i++)
     {
        int fd=events[i].data.fd;
        uint32_t revents=events[i].events;

        //先处理事故/挂起：直接跳过EPOLLIN/EPOLLOUT处理
        if(revents&(EPOLLHUP|EPOLLERR))
        {
            //先置空
        }

        //EPOLLIN有数据可读/新链接
        if(revents&EPOLLIN)
        {
            auto it=read_callbacks_.find(fd);
            if(it!=read_callbacks_.end()&&it->second)
            {
                it->second();
            }
        }

        //EPOLLOUT：可写
        if(revents&EPOLLOUT)
        {
            auto it=write_callbacks_.find(fd);
            if(it!=write_callbacks_.end()&&it->second)
            {
                it->second();
            }
        }
     }
    }
}


void EventLoop::quit()
{
    running_=false;
}

//=======epoll增删改查=========

void EventLoop::addFd(int fd,uint32_t events)
{
    struct epoll_event ev;
    ev.events=events|EPOLLET;//边缘触发
    ev.data.fd=fd;

    if(epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,fd,&ev)==-1)
    {
        throw std::runtime_error("epoll_ctl失败:"+std::string(strerror(errno)));
    }
}

void EventLoop::modifyFd(int fd,uint32_t events)
{
    struct epoll_event ev;
    ev.events=events|EPOLLET;
    ev.data.fd=fd;

    if(epoll_ctl(epoll_fd_,EPOLL_CTL_MOD,fd,&ev)==-1)
    {
        throw std::runtime_error("epoll_modify失败:"+std::string(strerror(errno)));
    }
}

void EventLoop::removed(int fd)
{
    //DEL一个不存在的fd会返回ENONT,这里忽略
    epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,fd,nullptr);
}

//===========注册回调=========

void EventLoop::setReadCallback(int fd,EventCallback cb)
{
    read_callbacks_[fd]=std::move(cb);
}

void EventLoop::setWriteCallback(int fd,EventCallback cb)
{
    write_callbacks_[fd]=std::move(cb);
}

void EventLoop::clearCallbacks(int fd)
{
    read_callbacks_.erase(fd);
    write_callbacks_.erase(fd);
}