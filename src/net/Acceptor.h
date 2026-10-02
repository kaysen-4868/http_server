#pragma once

#include <cstdint>
#include <functional>

class EventLoop;

class Acceptor {
public:
  //新链接回调：参数是accept出来的客户端fd
  using NewConnectionCallback = std::function<void(int)>;

  Acceptor(EventLoop *loop, int port);
  ~Acceptor();

  //禁止拷贝
  Acceptor(const Acceptor &) = delete;
  Acceptor &operator=(const Acceptor &) = delete;

  void setNewConnectionCallback(NewConnectionCallback cb);

  //开始监听:socket bind listen 加入epoll
  void listen();

  int listenfd() const { return listen_fd_; }

private:
  // listen_fd 可读时触发，循环accpet到EAGAIN
  void handleAccept();
  void handleEmfile();

  EventLoop *loop_;
  int listen_fd_;
  int port_;
  int idle_fd_; //预留的空闲fd
  NewConnectionCallback new_conn_cb_;
};