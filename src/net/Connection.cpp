#include "net/Connection.h"
#include "net/EventLoop.h"

#include<sys/epoll.h>
#include<unistd.h>
#include<cerrno>
#include<cstring>
#include<stdexcept>
#include<iostream>

static const int BUFFER_SIZE=4096;

//=========构造/析构============

Connection::Connection(EventLoop* loop,int fd)
: loop_(loop)
,fd_(fd)
,closed_(false)
{
    if(loop_==nullptr||fd_<0)
    {
        throw std::runtime_error("Connection:参数无效");
    }

    //注册进EVentloop
    loop_->addFd(fd_,EPOLLIN);//默认只关注可读

    
    loop_->setReadCallback(fd_,[this]()
{
    //如果对象已销毁，shared_from_this会抛异常，这里直接判断
    handleRead();
});

   loop_->setWriteCallback(fd_,[this]()
{
    handleWrite();
});
}

Connection::~Connection()
{
    //正常流程下close()已经把fd关闭了，这里只兜底
    if(!closed_)
    {
        loop_->removed(fd_);
        loop_->clearCallbacks(fd_);
        ::close(fd_);
        //  std::cout << "[析构] Connection fd=" << fd_ << " 被销毁" << std::endl;
    }
}

//==============上层接口============

void Connection::send(const std::string& data)
{
    if(closed_||data.empty())return;

    output_buffer_+=data;
    flushOutput();

}

void Connection::close()
{
    if(closed_)return;//防止重复close
    closed_=true;
//1.先从EventLoop摘掉，再关fd(顺序很重要)
    loop_->removed(fd_);
    loop_->clearCallbacks(fd_);
    ::close(fd_);

//2.通知上层
//注意close_cb_ 里上层可能erase掉conn的shared_ptr
//导致this析构。用shared_from_this()保护到函数结束 
if(close_cb_)
{
    auto self=shared_from_this();
    close_cb_(self);
}
}

//===========事件处理============

void Connection::handleRead()
{
    char buf[BUFFER_SIZE];

    while(true)
    {
        ssize_t n=::read(fd_,buf,sizeof(buf));

        if(n>0)
        {
            //追加到持续读缓存
            input_buffer_.append(buf,n);
        }
        else if(n==0)
        {
            //对端正常关闭
            std::cout<<"Connection:对端关闭fd="<<fd_<<"\n";
            close();
            return ;
        }
        else
        {
            if(errno==EAGAIN||errno==EWOULDBLOCK)
            {
                //ET模式:数据读干净
                break;
            }
            if(errno==EINTR)
            {
                continue;
            }
            std::cerr<<"Connection:read错误 fd="<<fd_<<":"<<strerror(errno)<<"\n";
            close();
            return ;
        }
    }

    //数据读完，通知上层处理（上层可能解析回显）
    if(!closed_&&message_cb_)
    {
        message_cb_(shared_from_this());
    }
}

void Connection::handleWrite()
{
    //EPOLLOUT触发:继续把output_buffer_发完
    flushOutput();
}

void Connection::flushOutput()
{
    while(!output_buffer_.empty())
    {
        ssize_t n=::write(fd_,output_buffer_.data(),output_buffer_.size());

        if(n>0)
        {
            //发出去多少删多少
            output_buffer_.erase(0,static_cast<size_t>(n));
        }
        else if(n==-1)
        {
            if(errno==EAGAIN||errno==EWOULDBLOCK)
            {
                //内核发送区满，加上EPOLLOUT等可写时再发
                loop_->modifyFd(fd_,EPOLLOUT|EPOLLIN);
                return;
            }
            if(errno==EINTR)
            {
                continue;
            }
            std::cerr<<"Connection:write错误 fd="<<fd_
            <<":"<<strerror(errno)<<"\n";
            close();
            return;
        }
    }

    loop_->modifyFd(fd_,EPOLLIN);
}



