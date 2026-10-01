#include "http/StaticFileHandler.h"
#include "http/MimeTypes.h"

#include<fstream>
#include<sstream>
#include<limits.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<iostream>

StaticFileHandler::StaticFileHandler(std::string root_dir)
:root_dir_(std::move(root_dir))
{
    //去掉末尾的/
    while(!root_dir_.empty()&&root_dir_.back()=='/')
    {
        root_dir_.pop_back();
    }
}

void StaticFileHandler::handle(const HttpRequest&req,HttpResponse& resp)
{
    auto setError=[&resp](int code,const std::string& msg)
    {
        resp.status_code=code;
        resp.status_message=msg;
        resp.headers["Content-Type"]="text/plain";
        resp.body=std::to_string(code)+" "+msg+"\n";
        resp.headers["Content-Length"]=std::to_string(resp.body.size());
    };

    //去掉query strig ?后面的部分
    std::string path=req.path;
    size_t q=path.find('?');
    if(q!=std::string::npos)path=path.substr(0,q);

    //目录请求  拼接index.html
    if(path.empty()||path.back()=='/')
    path+="index.html";

    //拼出完整句子
    std::string full_path=root_dir_+path;
    
    //用realpath规范化 校验是否 在根目录内
    char root_real[PATH_MAX];
    if(realpath(root_dir_.c_str(),root_real)==nullptr)
    {
        setError(500,"Internal Server Error");
        return;
    }

    char path_real[PATH_MAX];
    if(realpath(full_path.c_str(),path_real)==nullptr)
    {
        //文件不存在
        setError(404,"Not Found");
        return;
    }

    std::string real_path=path_real;
    std::string real_root=root_real;

    //前缀校验：必须完全匹配根目录前缀，且后面是'/'或结束
    //例如root="/www",path="/www2/x"不能通过
    if(real_path.compare(0,real_root.size(),real_root)!=0||
       (real_path.size()>real_root.size()&&real_path[real_root.size()]!='/'))
       {
        setError(403,"Forbidden");
        return ;
       }

     //如果指向目录，在尝试目录内的index.html
     struct stat st;
     if(stat(real_path.c_str(),&st)==0&& S_ISDIR(st.st_mode))
     {
        std::string index_path=real_path+"/index.html";
        if(stat(index_path.c_str(),&st)!=0||!S_ISREG(st.st_mode))
        {
            setError(404,"Not Found");
            return ;
        }
        real_path=index_path;
     }
     
     //读文件
     std::string content;
     if(!readFile(real_path,content))
     {
        setError(403,"Forbidden");
        return;
     }

     //填充相应
     resp.status_code=200;
     resp.status_message="OK";
     resp.headers["Content-Type"]=getMimeType(real_path);
     resp.body=std::move(content);

}

bool StaticFileHandler::readFile(const std::string& path,std::string& content)
{
    //必须binary模式，否则图片/二进制会被破坏
    std::ifstream ifs(path,std::ios::binary);
    if(!ifs)return false;

    std::ostringstream oss;
    oss<<ifs.rdbuf();
    content=oss.str();
    return true;
}