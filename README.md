# 简易静态 HTTP 服务器

一个用 C++17 从零实现的静态 HTTP 服务器，基于 epoll 边缘触发（ET）模型。

## 功能

- 支持 `GET` / `HEAD` 请求
- 支持 `200` / `400` / `403` / `404` / `405` / `413` / `501` 状态码
- 静态文件服务（HTML / CSS / JS / 图片 / 字体等）
- 常见 MIME 类型自动识别
- 路径安全校验（防目录穿越）
- 请求头大小限制（8KB）
- 单文件大小限制（10MB）
- 边缘触发 + 非阻塞 IO
- 处理 `SIGPIPE`、`EMFILE` 等边界情况

## 架构

```
                  ┌──────────────┐
                  │   main.cpp   │
                  └──────┬───────┘
                         │
            ┌────────────┼────────────┐
            │            │            │
            ▼            ▼            ▼
     ┌──────────┐  ┌──────────┐  ┌──────────────┐
     │EventLoop │  │ Acceptor │  │  HttpContext │
     │          │  │          │  │              │
     │ epoll    │  │ listen   │  │  HttpParser  │
     │ 事件分发 │  │ accept   │  │  状态机       │
     └────┬─────┘  └────┬─────┘  └──────────────┘
          │             │
          │             │ 创建
          │             ▼
          │      ┌──────────────┐
          └─────►│  Connection  │
                 │              │
                 │ 读写缓冲     │
                 │ 部分写处理   │
                 └──────┬───────┘
                        │ 回调
                        ▼
                 ┌──────────────┐
                 │ HttpHandler  │
                 │              │
                 │ StaticFile   │
                 │ Handler      │
                 └──────────────┘
```

**数据流**：

```
客户端请求
    ↓
[EventLoop] epoll_wait 返回
    ↓
[Acceptor] accept 新连接 → 创建 Connection
    ↓
[Connection] read 数据到 input_buffer_
    ↓
[HttpContext] HttpParser 状态机解析
    ↓
[HttpHandler] 调用 StaticFileHandler
    ↓
[StaticFileHandler] 读文件 + 设置 MIME
    ↓
[HttpResponse] 序列化成字节流
    ↓
[Connection] write 到 socket
    ↓
客户端响应
```

## 目录结构

```
http_server/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp
│   ├── net/
│   │   ├── EventLoop.h/.cpp
│   │   ├── Acceptor.h/.cpp
│   │   └── Connection.h/.cpp
│   ├── http/
│   │   ├── HttpRequest.h
│   │   ├── HttpResponse.h
│   │   ├── HttpParser.h/.cpp
│   │   ├── HttpContext.h
│   │   ├── HttpHandler.h
│   │   ├── MimeTypes.h
│   │   └── StaticFileHandler.h/.cpp
│   └── util/
│       └── Logger.h
└── www/
    ├── index.html
    ├── style.css
    ├── app.js
    └── logo.png
```

## 编译

依赖：
- CMake >= 3.10
- C++17 编译器（GCC 7+ / Clang 5+）
- Linux（依赖 epoll）

```bash
cmake -B build
cmake --build build
```

## 运行

```bash
./build/http_server
```

默认监听 `8080`，静态文件根目录为 `www/`。

浏览器打开 `http://127.0.0.1:8080/`。

## 测试

```bash
# 首页
curl -i http://127.0.0.1:8080/

# 404
curl -i http://127.0.0.1:8080/notexist

# 路径穿越（应 403）
curl -i --path-as-is http://127.0.0.1:8080/../../etc/passwd

# 方法不支持（应 501）
curl -i -X POST http://127.0.0.1:8080/

# 并发压测
ab -n 1000 -c 100 -k http://127.0.0.1:8080/
```

## 已知限制

- 不支持 HTTPS
- 不支持 POST / PUT / DELETE
- 不支持 `keep-alive`（每个请求一个连接）
- 不支持 gzip 压缩
- 不支持 Range 请求
- 大文件一次性读入内存（限制 10MB）
- 单 Reactor 单线程

## 技术要点

### 为什么用边缘触发（ET）

- 减少 `epoll_wait` 唤醒次数
- 必须配合非阻塞 IO，`read`/`write` 循环到 `EAGAIN`
- 配合持久缓冲区解决粘包/半包

### 粘包处理

- 每个连接一个 `input_buffer_`
- `HttpParser` 状态机从缓冲区消费
- 未消费的字节留下等下次数据

### 部分写处理

- 每个连接一个 `output_buffer_`
- `write` 返回 `EAGAIN` 时注册 `EPOLLOUT`
- 可写时继续 flush，直到发完

## License

MIT