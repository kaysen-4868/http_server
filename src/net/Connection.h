#pragma once

#include<cstdint>
#include<functional>
#include<memory>
#include<string>

class EventLoop;

class Connection:public std::enable_shared_from_this<Connection>
{
    public:
    using MessageCallback =std::function<void(const std::shared_ptr<Connection>&)>;
    using CloseCallback =std::function<void(const std::shared_ptr<Connection>&)>;

    Connection(EventLoop* loop,int fd);
    ~Connection();

    //禁止拷贝
    Connection(const Connection&)=delete;
    Connection& operator=(const Connection&)=delete;

    int fd() const{return fd_;}
    bool closed()const{return closed_;}

    //读写缓存区 对外可访问 方便上层解析
    std::string& inputBuffer(){return input_buffer_;}
    std::string& outputBuffer(){return output_buffer_;}

    void setMessageCallback(MessageCallback cb) { message_cb_ = std::move(cb); }
    void setCloseCallback(CloseCallback cb)     { close_cb_   = std::move(cb); }

    //上层调用 追加数据到发送缓存区中，并尝试发送
    void send(const std::string&data);

    //主动关闭连接
    void close();

    bool close_after_write_=false;
    void closeAfterWrite()
    {
        close_after_write_=true;
        if(output_buffer_.empty())close();
    }

    private:
    void handleRead();//EPOLLIN触发 读到EAGAIN，追加到input_buffer_，回调
    void handleWrite();//EPOLLOUT触发 继续发out_buffer_
    void flushOutput();//实际write 处理部分写+EAGAIN

    EventLoop* loop_;
    int fd_;
    bool closed_;

    std::string input_buffer_;//持久读缓存。解决粘包问题
    std::string output_buffer_;//持久写缓存，

    MessageCallback message_cb_;
    CloseCallback close_cb_;
};
