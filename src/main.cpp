#include "net/EventLoop.h"
#include "net/Acceptor.h"
#include "net/Connection.h"
#include "http/HttpContext.h"//每条连接的HTTP解析上下文
#include "http/HttpResponse.h"//HTTP响应对象
#include "http/HttpHandler.h"//业务层:根据请求生成响应
#include "http/StaticFileHandler.h"

#include<unordered_map>
#include<memory>
#include<iostream>

int main()
{

  try
  {
    //创建事件循环加监听器
    EventLoop loop;
    Acceptor acceptor(&loop,8080);

    auto file_handler =std::make_shared<StaticFileHandler>("www");
    //全局连接表
    //用shared_ptr<map>是为了让多个Lamda共享同一份表
    //如果用值捕获，则改变的只是副本，无法互相同步
    auto conns=std::make_shared<std::unordered_map<int,std::shared_ptr<Connection>>>();
    auto  contexts=std::make_shared<std::unordered_map<int,std::shared_ptr<HttpContext>>>();

    //注册新链接到来的回调
    //没accept到一个新的fd，就会调用对应的lambda
    acceptor.setNewConnectionCallback(
      [&loop,conns,contexts,file_handler](int client_fd)
      {
        //为新链接创建Connection对象(shared_ptr持有，因为回调要长期持有)
        auto conn=std::make_shared<Connection>(&loop,client_fd);
        (*contexts)[client_fd]=std::make_shared<HttpContext>();
        //为这条连接创建HTTP解析上下文
        conn->setMessageCallback(
          [contexts,file_handler](const std::shared_ptr<Connection>&c)
          {
            //找到这条连接对应的解析上下文
            auto it=contexts->find(c->fd());
            if(it==contexts->end())return ;
            auto& ctx=it->second;

            //把读缓冲区的数据喂给解析器
            //返回true表示解析出一条完整请求
            bool complete=ctx->parseRequest(c->inputBuffer());
            
            //情况A:解析出错 返回400并关闭连接
            if(ctx->hasError())
            {
              HttpResponse resp;
              resp.status_code=400;
              resp.status_message="Bad Request";
              resp.headers["Content-Type"]="text/plain";
              resp.body="400 Bad Request\n";
              c->send(resp.toString());
              c->close();
              return ;

            }

            //情况B:数据还没收完  直接返回 等下次read
            if(!complete)return ;

            //情况C: 解析成功 交给业务层处理
            const HttpRequest& req=ctx->request();
            std::cout<<"请求:"<<req.method<<" "
            <<req.path<<" "<<req.version<<"\n";

            //业务层根据请求生成响应
            HttpResponse resp;
            handleHttpRequest(req,resp,*file_handler);
            c->send(resp.toString());

            //关键 重置解析上下文 准备解析下一条请求
            //HTTP/1.1 keep-alive 下 同一链接可能有多条请求
            ctx->reset();
          });

          //注册连接关闭回调 
          conn->setCloseCallback(
            [conns,contexts](const std::shared_ptr<Connection>&c)
            {
              //从两张表里移除这条连接
              conns->erase(c->fd());
              contexts->erase(c->fd());
              std::cout<<"连接已清理fd="<<c->fd()<<"\n";
            }
          );

          //把新链接加入全局表
          //回调全部注册完后，最后再放进map
          //避免回调还没装好，事件就触发了
          (*conns)[client_fd]=conn;
      });

      //启动监听并进入事件循环
      acceptor.listen();
      std::cout<<"HTTP服务器启动,监听8080\n";
      loop.loop();//阻塞直到服务器结束
  }
  catch(const std::exception& e)
  {
    std::cerr<<"错误:"<<e.what()<<"\n";
    return 1;
  }

    return 0;
}