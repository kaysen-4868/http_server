#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

class EventLoop {
public:
  using EventCallback = std::function<void()>;

  EventLoop();
  ~EventLoop();
  //禁止拷贝
  EventLoop(const EventLoop &) = delete;
  EventLoop &operator=(const EventLoop &) = delete;

  //主循环：epoll_wait+分发事件
  void loop();
  void quit();

  //增删改查关注的事件
  void addFd(int fd, uint32_t events);
  void modifyFd(int fd, uint32_t events);
  void removed(int fd);

  //注册回调
  void setReadCallback(int fd, EventCallback cb);
  void setWriteCallback(int fd, EventCallback cb);
  void clearCallbacks(int fd);

  int epollfd() const { return epoll_fd_; }

private:
  static const int MAX_EVENTS = 1024;

  int epoll_fd_;
  bool running_;

  // fd+回调
  std::unordered_map<int, EventCallback> read_callbacks_;
  std::unordered_map<int, EventCallback> write_callbacks_;
};